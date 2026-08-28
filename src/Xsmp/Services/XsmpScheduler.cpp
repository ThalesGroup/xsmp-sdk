// Copyright 2023 THALES ALENIA SPACE FRANCE. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#include <Smp/IComponent.h>
#include <Smp/IComposite.h>
#include <Smp/IEntryPoint.h>
#include <Smp/ISimulator.h>
#include <Smp/IStorageReader.h>
#include <Smp/IStorageWriter.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/EventId.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Smp/Services/TimeKind.h>
#include <Smp/SimulatorStateKind.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Persist.h>
#include <Xsmp/Persist/SmpIObject.h>
#include <Xsmp/Persist/StdMap.h>
#include <Xsmp/Persist/StdSet.h>
#include <Xsmp/Persist/StdString.h>
#include <Xsmp/Persist/StdVector.h>
#include <Xsmp/Services/XsmpScheduler.h>
#include <Xsmp/Services/XsmpSchedulerGen.h>
#include <algorithm>
#include <chrono>
#include <iterator>
#include <limits>
#include <mutex>
#include <thread>
#include <utility>

//  helpers to persist XsmpScheduler::Event type
namespace Xsmp::Persist {

template <> struct Helper<::Xsmp::Services::XsmpScheduler::Event> {
  static void Store(const ::Smp::ISimulator *simulator,
                    ::Smp::IStorageWriter *writer,
                    const ::Xsmp::Services::XsmpScheduler::Event &value) {
    ::Xsmp::Persist::Store(simulator, writer, value.entryPoint,
                           value.nextScheduleSimulationTime, value.time,
                           value.cycleTime, value.repeat, value.kind);
  }
  static void Restore(const ::Smp::ISimulator *simulator,
                      ::Smp::IStorageReader *reader,
                      ::Xsmp::Services::XsmpScheduler::Event &value) {
    ::Xsmp::Persist::Restore(simulator, reader, value.entryPoint,
                             value.nextScheduleSimulationTime, value.time,
                             value.cycleTime, value.repeat, value.kind);
  }
};
} // namespace Xsmp::Persist

namespace Xsmp::Services {
static constexpr ::Smp::Duration MaxDuration =
    std::numeric_limits<::Smp::Duration>::max();
static constexpr ::Smp::Duration MinDuration =
    std::numeric_limits<::Smp::Duration>::lowest();

namespace {
/// Add two durations, saturating at the limits of the type instead of
/// overflowing, which would be undefined behavior and would silently schedule
/// the event in the past.
constexpr ::Smp::Duration add(::Smp::Duration lhs,
                              ::Smp::Duration rhs) noexcept {
  if (rhs > 0 && lhs > MaxDuration - rhs) {
    return MaxDuration;
  }
  if (rhs < 0 && lhs < MinDuration - rhs) {
    return MinDuration;
  }
  return lhs + rhs;
}
/// Subtract two durations, saturating at the limits of the type.
constexpr ::Smp::Duration subtract(::Smp::Duration lhs,
                                   ::Smp::Duration rhs) noexcept {
  if (rhs > 0 && lhs < MinDuration + rhs) {
    return MinDuration;
  }
  if (rhs < 0 && lhs > MaxDuration + rhs) {
    return MaxDuration;
  }
  return lhs - rhs;
}
} // namespace

XsmpScheduler::XsmpScheduler(::Smp::String8 name, ::Smp::String8 description,
                             ::Smp::IComposite *parent,
                             ::Smp::ISimulator *simulator)
    : XsmpSchedulerGen::XsmpSchedulerGen(name, description, parent, simulator) {

  // post an event to Hold the simulation at the maximal duration
  static constexpr ::Smp::Services::EventId holdId = -2;
  _events.try_emplace(holdId,
                      Event{&HoldEvent, MaxDuration, MaxDuration, 0, 0,
                            ::Smp::Services::TimeKind::TK_SimulationTime});
  _events_table.try_emplace(MaxDuration, EventList{holdId});
}

XsmpScheduler::~XsmpScheduler() {
  // recall DoDisconnect to ensure zulu thread is correctly stopped
  DoDisconnect();
}

void XsmpScheduler::DoConnect(const ::Smp::ISimulator *simulator) {

  simulator->GetEventManager()->Subscribe(
      ::Smp::Services::IEventManager::SMP_EnterExecutingId, &EnterExecuting);
  simulator->GetEventManager()->Subscribe(
      ::Smp::Services::IEventManager::SMP_LeaveExecutingId, &LeaveExecuting);
  simulator->GetEventManager()->Subscribe(
      ::Smp::Services::IEventManager::SMP_EnterAbortingId, &_abort);
  simulator->GetEventManager()->Subscribe(
      ::Smp::Services::IEventManager::SMP_EpochTimeChangedId,
      &_epochTimeChanged);
  simulator->GetEventManager()->Subscribe(
      ::Smp::Services::IEventManager::SMP_MissionTimeChangedId,
      &_missionTimeChanged);

  _zuluThread = std::thread(&XsmpScheduler::InternalZuluRun, this);
}

void XsmpScheduler::DoDisconnect() {
  // stop the zulu thread
  {
    const std::scoped_lock lck{_zuluEventsTableMutex};
    // if already terminated do nothing
    if (_terminate) {
      return;
    }
    _terminate = true;
  }
  _zuluCv.notify_one();
  if (_zuluThread.joinable()) {
    _zuluThread.join();
  }
}
void XsmpScheduler::SetTargetSpeed(double speed) {
  if (speed < 0.01) {
    _targetSpeed = 0.01;
  } else if (speed > 100.) {
    _targetSpeed = 100.;
  } else {
    _targetSpeed = speed;
  }
}

double XsmpScheduler::GetTargetSpeed() const noexcept { return _targetSpeed; }

::Smp::Services::EventId
XsmpScheduler::AddImmediateEvent(const ::Smp::IEntryPoint *entryPoint) {

  const std::scoped_lock lck{_eventsMutex};
  ++_lastEventId;
  auto time = GetSimulator()->GetTimeKeeper()->GetSimulationTime();
  _events.try_emplace(_lastEventId,
                      Event{entryPoint, time, time, 0, 0,
                            ::Smp::Services::TimeKind::TK_SimulationTime});

  _immediate_events.emplace(_lastEventId);
  return _lastEventId;
}

::Smp::Services::EventId
XsmpScheduler::AddEvent(const ::Smp::IEntryPoint *entryPoint,
                        ::Smp::Duration simulationTime, ::Smp::Duration time,
                        ::Smp::Duration cycleTime, ::Smp::Int64 repeat,
                        ::Smp::Services::TimeKind kind) {

  if (auto currentSimulationTime =
          GetSimulator()->GetTimeKeeper()->GetSimulationTime();
      simulationTime < currentSimulationTime) {
    ::Xsmp::Exception::throwInvalidEventTime(this, simulationTime,
                                             currentSimulationTime);
  }
  if (repeat != 0 && cycleTime <= 0) {
    ::Xsmp::Exception::throwInvalidCycleTime(this, cycleTime);
  }
  const std::scoped_lock lck{_eventsMutex};

  ++_lastEventId;
  _events.try_emplace(_lastEventId, Event{entryPoint, simulationTime, time,
                                          cycleTime, repeat, kind});

  _events_table.try_emplace(simulationTime).first->second.emplace(_lastEventId);

  GetSimulator()->GetLogger()->Log(entryPoint, "Event posted",
                                   ::Smp::Services::ILogger::LMK_Debug);

  return _lastEventId;
}

::Smp::Services::EventId XsmpScheduler::AddSimulationTimeEvent(
    const ::Smp::IEntryPoint *entryPoint, ::Smp::Duration simulationTime,
    ::Smp::Duration cycleTime, ::Smp::Int64 repeat) {

  const ::Smp::Duration time =
      add(GetSimulator()->GetTimeKeeper()->GetSimulationTime(), simulationTime);
  return AddEvent(entryPoint, time, time, cycleTime, repeat,
                  ::Smp::Services::TimeKind::TK_SimulationTime);
}

::Smp::Services::EventId XsmpScheduler::AddMissionTimeEvent(
    const ::Smp::IEntryPoint *entryPoint, ::Smp::Duration missionTime,
    ::Smp::Duration cycleTime, ::Smp::Int64 repeat) {

  auto simulationTime = subtract(
      add(GetSimulator()->GetTimeKeeper()->GetSimulationTime(), missionTime),
      GetSimulator()->GetTimeKeeper()->GetMissionTime());

  return AddEvent(entryPoint, simulationTime, missionTime, cycleTime, repeat,
                  ::Smp::Services::TimeKind::TK_MissionTime);
}

::Smp::Services::EventId XsmpScheduler::AddEpochTimeEvent(
    const ::Smp::IEntryPoint *entryPoint, ::Smp::DateTime epochTime,
    ::Smp::Duration cycleTime, ::Smp::Int64 repeat) {

  auto simulationTime = subtract(
      add(GetSimulator()->GetTimeKeeper()->GetSimulationTime(), epochTime),
      GetSimulator()->GetTimeKeeper()->GetEpochTime());

  return AddEvent(entryPoint, simulationTime, epochTime, cycleTime, repeat,
                  ::Smp::Services::TimeKind::TK_EpochTime);
}

::Smp::Services::EventId XsmpScheduler::AddZuluTimeEvent(
    const ::Smp::IEntryPoint *entryPoint, ::Smp::DateTime zuluTime,
    ::Smp::Duration cycleTime, ::Smp::Int64 repeat) {

  // check zulu time not in the past
  if (auto currentZulu = GetSimulator()->GetTimeKeeper()->GetZuluTime();
      zuluTime < currentZulu) {
    ::Xsmp::Exception::throwInvalidEventTime(this, zuluTime, currentZulu);
  }
  if (repeat != 0 && cycleTime <= 0) {
    ::Xsmp::Exception::throwInvalidCycleTime(this, cycleTime);
  }
  ::Smp::Services::EventId eventId = -1;
  {
    // create the event
    // taken in the order documented in the header, as RemoveEvent() does
    const std::scoped_lock eventsLck{_eventsMutex};
    const std::scoped_lock zuluLck{_zuluEventsTableMutex};
    eventId = ++_lastEventId;
    _events.try_emplace(eventId,
                        Event{entryPoint, zuluTime, zuluTime, cycleTime, repeat,
                              ::Smp::Services::TimeKind::TK_ZuluTime});

    // insert the event in the zulu event table
    _zulu_events_table.try_emplace(zuluTime).first->second.emplace(eventId);
  }
  _zuluCv.notify_one();

  GetSimulator()->GetLogger()->Log(entryPoint, "Event posted",
                                   ::Smp::Services::ILogger::LMK_Debug);
  return eventId;
}

void XsmpScheduler::SetEventTime(::Smp::Services::EventId eventId,
                                 ::Smp::Duration simulationTime,
                                 ::Smp::Duration time,
                                 ::Smp::Services::TimeKind kind) {

  const std::scoped_lock lck{_eventsMutex};
  auto it = _events.find(eventId);

  if (it == _events.end() || it->second.kind != kind) {
    ::Xsmp::Exception::throwInvalidEventId(this, eventId);
  }
  if (auto entry = _events_table.find(it->second.nextScheduleSimulationTime);
      entry != _events_table.end()) {
    entry->second.erase(eventId);
    if (entry->second.empty()) {
      _events_table.erase(entry);
    }
  }
  if (simulationTime < GetSimulator()->GetTimeKeeper()->GetSimulationTime()) {
    // the event is in the past: remove it, unless it is the event currently
    // being executed, whose entry is still in use by Execute()
    if (_currentEventId == eventId) {
      it->second.repeat = 0;
    } else {
      _events.erase(it);
    }
    return;
  }

  it->second.nextScheduleSimulationTime = simulationTime;
  it->second.time = time;

  _events_table.try_emplace(simulationTime).first->second.emplace(eventId);
}

void XsmpScheduler::SetEventSimulationTime(::Smp::Services::EventId event,
                                           ::Smp::Duration simulationTime) {

  const ::Smp::Duration time =
      add(GetSimulator()->GetTimeKeeper()->GetSimulationTime(), simulationTime);
  SetEventTime(event, time, time, ::Smp::Services::TimeKind::TK_SimulationTime);
}

void XsmpScheduler::SetEventMissionTime(::Smp::Services::EventId event,
                                        ::Smp::Duration missionTime) {
  const ::Smp::Duration simulationTime = subtract(
      add(GetSimulator()->GetTimeKeeper()->GetSimulationTime(), missionTime),
      GetSimulator()->GetTimeKeeper()->GetMissionTime());
  SetEventTime(event, simulationTime, missionTime,
               ::Smp::Services::TimeKind::TK_MissionTime);
}

void XsmpScheduler::SetEventEpochTime(::Smp::Services::EventId event,
                                      ::Smp::DateTime epochTime) {
  const ::Smp::Duration simulationTime = subtract(
      add(GetSimulator()->GetTimeKeeper()->GetSimulationTime(), epochTime),
      GetSimulator()->GetTimeKeeper()->GetEpochTime());
  SetEventTime(event, simulationTime, epochTime,
               ::Smp::Services::TimeKind::TK_EpochTime);
}

void XsmpScheduler::SetEventZuluTime(::Smp::Services::EventId event,
                                     ::Smp::DateTime zuluTime) {
  {
    auto currentZulu = GetSimulator()->GetTimeKeeper()->GetZuluTime();
    // taken in the order documented in the header, as RemoveEvent() does
    const std::scoped_lock eventsLck{_eventsMutex};
    const std::scoped_lock zuluLck{_zuluEventsTableMutex};
    auto it = _events.find(event);

    if (it == _events.end() ||
        it->second.kind != ::Smp::Services::TimeKind::TK_ZuluTime) {
      ::Xsmp::Exception::throwInvalidEventId(this, event);
    }
    if (auto entry =
            _zulu_events_table.find(it->second.nextScheduleSimulationTime);
        entry != _zulu_events_table.end()) {
      entry->second.erase(event);
      if (entry->second.empty()) {
        _zulu_events_table.erase(entry);
      }
    }

    if (zuluTime < currentZulu) {
      // the zulu time is in the past: remove the event, unless it is the one
      // currently being executed, whose entry is still in use by ExecuteZulu()
      if (_currentEventId == event) {
        it->second.repeat = 0;
      } else {
        _events.erase(it);
      }
      return;
    }

    it->second.nextScheduleSimulationTime = zuluTime;
    _zulu_events_table.try_emplace(zuluTime).first->second.emplace(event);
  }
  _zuluCv.notify_one();
}

void XsmpScheduler::SetEventCycleTime(::Smp::Services::EventId event,
                                      ::Smp::Duration cycleTime) {

  const std::scoped_lock lck{_eventsMutex};
  auto it = _events.find(event);
  if (it == _events.end()) {
    ::Xsmp::Exception::throwInvalidEventId(this, event);
  }
  // an event is cyclic as soon as its repeat is not 0, -1 included
  if (it->second.repeat != 0 && cycleTime <= 0) {
    ::Xsmp::Exception::throwInvalidCycleTime(this, cycleTime);
  }
  it->second.cycleTime = cycleTime;
}

void XsmpScheduler::SetEventRepeat(::Smp::Services::EventId event,
                                   ::Smp::Int64 repeat) {

  const std::scoped_lock lck{_eventsMutex};
  auto it = _events.find(event);

  if (it == _events.end()) {
    ::Xsmp::Exception::throwInvalidEventId(this, event);
  }
  if (repeat != 0 && it->second.cycleTime <= 0) {
    ::Xsmp::Exception::throwInvalidCycleTime(this, it->second.cycleTime);
  }
  it->second.repeat = repeat;
}

void XsmpScheduler::RemoveEvent(::Smp::Services::EventId event) {

  const std::scoped_lock lck{_eventsMutex};
  auto it = _events.find(event);
  if (it == _events.end()) {
    ::Xsmp::Exception::throwInvalidEventId(this, event);
  }
  if (_currentEventId == event) {
    it->second.repeat = 0;
    return;
  }

  const auto time = it->second.nextScheduleSimulationTime;
  if (it->second.kind == ::Smp::Services::TimeKind::TK_ZuluTime) {
    const std::scoped_lock lckZulu{_zuluEventsTableMutex};
    if (auto entry = _zulu_events_table.find(time);
        entry != _zulu_events_table.end()) {
      entry->second.erase(event);
      if (entry->second.empty()) {
        _zulu_events_table.erase(entry);
      }
    }
  } else if (auto entry = _events_table.find(time);
             entry != _events_table.end()) {
    entry->second.erase(event);
    if (entry->second.empty()) {
      _events_table.erase(entry);
    }
  }
  _events.erase(it);
}

::Smp::Services::EventId XsmpScheduler::GetCurrentEventId() const {
  const std::scoped_lock lck{_eventsMutex};
  return _currentEventId;
}

::Smp::Duration XsmpScheduler::GetNextScheduledEventTime() const {
  const std::scoped_lock lck{_eventsMutex};
  if (_events_table.begin() != _events_table.end()) {
    return _events_table.begin()->first;
  }
  return std::numeric_limits<::Smp::Duration>::max();
}

void XsmpScheduler::Execute(::Smp::Services::EventId eventId) {

  // _execMutex is taken first and kept for the whole execution: it serializes
  // the execution of the events between the simulation and the zulu threads.
  // Taking it before _eventsMutex is mandatory, see the lock order documented
  // in the header.
  const std::scoped_lock execLck{_execMutex};
  std::unique_lock lck{_eventsMutex};

  auto it = _events.find(eventId);
  if (it == _events.end()) {
    // the event has been removed in the meantime
    return;
  }

  // the event may have been rescheduled to a later simulation time while the
  // events of the current time were being executed, by a change of the epoch or
  // mission time: it is already posted in the scheduling table at its new time
  if (it->second.nextScheduleSimulationTime >
      GetSimulator()->GetTimeKeeper()->GetSimulationTime()) {
    return;
  }

  const auto *entryPoint = it->second.entryPoint;
  _currentEventId = eventId;
  lck.unlock();
  ::Xsmp::Helper::SafeExecute(GetSimulator(), entryPoint);
  lck.lock();
  _currentEventId = -1;

  // the entry point may have modified the scheduling table
  it = _events.find(eventId);
  if (it == _events.end()) {
    return;
  }
  auto &event = it->second;

  if (event.repeat == 0) {
    // remove the event
    _events.erase(it);
  } else {
    // decrement the repeat
    if (event.repeat > 0) {
      event.repeat--;
    }
    // compute the next time the event will be executed
    event.nextScheduleSimulationTime =
        add(event.nextScheduleSimulationTime, event.cycleTime);
    event.time = add(event.time, event.cycleTime);

    // post the event in the table
    _events_table.try_emplace(event.nextScheduleSimulationTime)
        .first->second.emplace(eventId);
  }
}

void XsmpScheduler::ExecuteZulu(::Smp::Services::EventId eventId) {

  const std::scoped_lock execLck{_execMutex};
  std::unique_lock lck{_eventsMutex};

  auto it = _events.find(eventId);
  if (it == _events.end()) {
    // the event has been removed in the meantime
    return;
  }

  // execute the event only in executing and standby states
  if (auto state = GetSimulator()->GetState();
      state == ::Smp::SimulatorStateKind::SSK_Executing ||
      state == ::Smp::SimulatorStateKind::SSK_Standby) {

    const auto *entryPoint = it->second.entryPoint;
    _currentEventId = eventId;
    lck.unlock();
    ::Xsmp::Helper::SafeExecute(GetSimulator(), entryPoint);
    lck.lock();
    _currentEventId = -1;

    // the entry point may have modified the scheduling table
    it = _events.find(eventId);
    if (it == _events.end()) {
      return;
    }
  }
  auto &event = it->second;

  if (event.repeat == 0) {
    // remove the event
    _events.erase(it);
  } else {
    // decrement the repeat
    if (event.repeat > 0) {
      event.repeat--;
    }
    // compute the next time the event will be executed
    event.nextScheduleSimulationTime =
        add(event.nextScheduleSimulationTime, event.cycleTime);
    // post the event in the table
    const std::scoped_lock lckZulu{_zuluEventsTableMutex};
    _zulu_events_table.try_emplace(event.nextScheduleSimulationTime)
        .first->second.emplace(eventId);
  }
}

bool XsmpScheduler::ExecuteEvents(::Smp::Duration time) {

  // execute all the events scheduled at that time, including the ones posted
  // while executing them
  while (true) {
    EventList current;
    {
      const std::scoped_lock lck{_eventsMutex};
      // the entry is taken out of the table: an event posted at the same time
      // while executing these ones creates a new entry
      auto node = _events_table.extract(time);
      if (!node || node.mapped().empty()) {
        return true;
      }
      current = std::move(node.mapped());
    }

    for (auto it = current.begin(); it != current.end(); ++it) {
      Execute(*it);
      // process immediate events posted by this event
      if (_simulationStatus == Status::Hold || !ExecuteImmediateEvents()) {
        // put the events that have not been executed back in the table
        if (auto next = std::next(it); next != current.end()) {
          const std::scoped_lock lck{_eventsMutex};
          _events_table[time].insert(next, current.end());
        }
        return false;
      }
    }
  }
}

bool XsmpScheduler::ExecuteImmediateEvents() {
  // execute all events
  while (true) {
    EventList events;
    {
      const std::scoped_lock lck{_eventsMutex};
      if (_immediate_events.empty()) {
        return true;
      }
      // swap the current list of events
      events.swap(_immediate_events);
    }

    for (auto it = events.begin(); it != events.end(); ++it) {
      Execute(*it);
      // stop here if a hold has been requested by this event
      if (_simulationStatus == Status::Hold) {
        // store un-executed events and exit
        const std::scoped_lock lck{_eventsMutex};
        _immediate_events.insert(std::next(it), events.end());
        return false;
      }
    }
  }
}

void XsmpScheduler::InternalZuluRun() {

  std::unique_lock lck(_zuluEventsTableMutex);

  while (!_terminate) {

    // execute all events with time <= current zulu time
    for (auto it = _zulu_events_table.begin();
         it != _zulu_events_table.end() &&
         it->first <= GetSimulator()->GetTimeKeeper()->GetZuluTime();) {

      // execute all events
      while (!it->second.empty()) {
        // swap the event list
        EventList events;
        events.swap(it->second);

        // if _zulu_events_table is modified while executing the events,
        // the implementation guarantee that "it" remains valid and only element
        // after "it" could be appended
        for (auto eventId : events) {
          lck.unlock();
          ExecuteZulu(eventId);
          lck.lock();
          // stop here if terminate signal received during event execution
          if (_terminate) {
            return;
          }
        }
      }
      _zulu_events_table.erase(it);
      it = _zulu_events_table.begin();
    }

    // wait until next event
    if (_zulu_events_table.empty()) {
      // exit if terminate signal received or a new event is posted.
      // in case a new event is posted, exit the loop and wait for the remaining
      // time (else branch)
      _zuluCv.wait(
          lck, [this]() { return _terminate || !_zulu_events_table.empty(); });
    } else {
      // no predicate here: an event posted while waiting can be due before the
      // one this delay was computed from, and the loop recomputes the delay
      _zuluCv.wait_for(
          lck, std::chrono::nanoseconds{std::max(
                   static_cast<::Smp::Duration>(0),
                   _zulu_events_table.begin()->first -
                       GetSimulator()->GetTimeKeeper()->GetZuluTime())});
    }
  }
}

void XsmpScheduler::Restore(::Smp::IStorageReader *reader) {
  const std::scoped_lock lck{_eventsMutex};
  std::map<::Smp::Services::EventId, Event> events;
  ::Smp::Services::EventId lastEventId = -1;
  ::Xsmp::Persist::Restore(GetSimulator(), this, reader, events, _events_table,
                           _immediate_events, lastEventId);

  // the events scheduled on zulu time are not part of the persisted state:
  // they are kept as they are, and the identifiers they use stay allocated
  for (const auto &[eventId, event] : _events) {
    if (event.kind == ::Smp::Services::TimeKind::TK_ZuluTime) {
      events.insert_or_assign(eventId, event);
      lastEventId = std::max(lastEventId, eventId);
    }
  }
  _events = std::move(events);
  _lastEventId = lastEventId;
}

void XsmpScheduler::Store(::Smp::IStorageWriter *writer) {
  const std::scoped_lock lck{_eventsMutex};
  std::map<::Smp::Services::EventId, Event> events;
  for (const auto &[eventId, event] : _events) {
    if (event.kind != ::Smp::Services::TimeKind::TK_ZuluTime) {
      events.emplace(eventId, event);
    }
  }
  ::Xsmp::Persist::Store(GetSimulator(), this, writer, events, _events_table,
                         _immediate_events, _lastEventId);
}

void XsmpScheduler::RescheduleEvents(::Smp::Services::TimeKind kind,
                                     ::Smp::Duration newTime) {

  const std::scoped_lock lck{_eventsMutex};
  const auto offset =
      subtract(newTime, GetSimulator()->GetTimeKeeper()->GetSimulationTime());

  for (auto it = _events.begin(); it != _events.end();) {
    auto &event = it->second;
    if (event.kind != kind) {
      ++it;
      continue;
    }
    // an occurrence before the new time is not executed and consumes a repeat
    while (event.time < newTime && event.repeat != 0) {
      if (event.repeat > 0) {
        --event.repeat;
      }
      event.time = add(event.time, event.cycleTime);
    }
    // the event is posted again below, at the simulation time matching the new
    // offset; the event being executed is posted again by Execute() instead
    const auto isCurrent = _currentEventId == it->first;
    if (auto entry = _events_table.find(event.nextScheduleSimulationTime);
        entry != _events_table.end()) {
      entry->second.erase(it->first);
      if (entry->second.empty()) {
        _events_table.erase(entry);
      }
    }

    if (event.time < newTime) {
      // every remaining occurrence is in the past
      if (isCurrent) {
        event.repeat = 0;
        ++it;
      } else {
        it = _events.erase(it);
      }
      continue;
    }

    event.nextScheduleSimulationTime = subtract(event.time, offset);
    if (!isCurrent) {
      _events_table.try_emplace(event.nextScheduleSimulationTime)
          .first->second.emplace(it->first);
    }
    ++it;
  }
}

void XsmpScheduler::_LeaveExecuting() {

  {
    const std::scoped_lock lck{_holdMutex};
    _simulationStatus = Status::Hold;
  }
  _holdCv.notify_one();
}

void XsmpScheduler::_EnterExecuting() {

  auto *timeKeeper = GetSimulator()->GetTimeKeeper();
  auto startZuluTime = timeKeeper->GetZuluTime();

  _simulationStatus = Status::Running;

  _load.clear();
  _speed.clear();

  ::Smp::Duration delay = 0;

  // process all immediate events
  if (!ExecuteImmediateEvents()) {
    return; // exit immediately in case of hold
  }
  auto *eventManager = GetSimulator()->GetEventManager();

  // execute all events
  while (true) {
    ::Smp::Duration time;
    {
      const std::scoped_lock lck{_eventsMutex};
      auto it = _events_table.begin();
      if (it == _events_table.end()) {
        break;
      }
      time = it->first;
    }

    // notify that simulation time will be changed
    eventManager->Emit(::Smp::Services::IEventManager::SMP_PreSimTimeChangeId,
                       false);

    if (_simulationStatus == Status::Hold) {
      return; // exit immediately if hold is requested
    }
    auto duration = subtract(time, timeKeeper->GetSimulationTime());
    delay += static_cast<::Smp::Duration>(static_cast<double>(duration) /
                                          _targetSpeed) -
             (timeKeeper->GetZuluTime() - startZuluTime);

    auto endZuluTime = timeKeeper->GetZuluTime();
    // update speed
    if (duration) {
      _speed.AddSample(static_cast<double>(endZuluTime - startZuluTime) /
                       static_cast<double>(duration));
    }
    startZuluTime = endZuluTime;

    // keep synchronized with zulu time
    if (delay > 0) {

      if (std::unique_lock lck2{_holdMutex};
          _holdCv.wait_for(lck2, std::chrono::nanoseconds{delay}, [this] {
            // continue to wait while Hold is not requested
            return _simulationStatus == Status::Hold;
          })) {
        return; // exit immediately in case of hold
      }
    }

    // change the simulation time
    timeKeeper->SetSimulationTime(time);

    // notify that simulation time has changed
    eventManager->Emit(::Smp::Services::IEventManager::SMP_PostSimTimeChangeId,
                       false);

    // process all events (check for eventually events added by
    // SMP_PostSimTimeChangeId )
    if (!ExecuteImmediateEvents() || !ExecuteEvents(time)) {
      return; // exit immediately in case of hold
    }
  }

  _load.clear();
  _speed.clear();
}

void XsmpScheduler::_HoldEvent() {
  // return in standby state
  GetSimulator()->Hold(true);
}

void XsmpScheduler::MovingAverage::AddSample(double sample) {
  const std::scoped_lock lck{_mutex};
  sum = sum + sample - samples[index];
  samples[index] = sample;
  ++index;
  if (index == sampleCount) {
    index = 0;
  }
  if (size < sampleCount) {
    ++size;
  }
}
double XsmpScheduler::MovingAverage::GetAverage() {
  const std::scoped_lock lck{_mutex};
  return size ? sum / size : 0.;
}
void XsmpScheduler::MovingAverage::clear() {
  const std::scoped_lock lck{_mutex};
  size = 0;
  for (auto &sample : samples) {
    sample = 0.;
  }
  index = 0;
  sum = 0.;
}
} // namespace Xsmp::Services
