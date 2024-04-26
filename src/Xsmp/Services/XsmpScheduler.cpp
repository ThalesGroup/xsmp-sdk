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

#include <Smp/IEntryPoint.h>
#include <Smp/ISimulator.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Smp/SimulatorStateKind.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Persist/SmpIObject.h>
#include <Xsmp/Persist/StdMap.h>
#include <Xsmp/Persist/StdSet.h>
#include <Xsmp/Persist/StdString.h>
#include <Xsmp/Persist/StdVector.h>
#include <Xsmp/Services/XsmpScheduler.h>
#include <algorithm>
#include <cassert>
#include <chrono>
#include <limits>

//  helpers to persist XsmpScheduler::Event type
namespace Xsmp::Persist {

template <> struct Helper<::Xsmp::Services::XsmpScheduler::Event> {
  static void Store(const ::Smp::ISimulator *simulator,
                    ::Smp::IStorageWriter *writer,
                    const ::Xsmp::Services::XsmpScheduler::Event &value) {
    ::Xsmp::Persist::Store(simulator, writer, value.entryPoint,
                           value.nextScheduleSimulationTime, value.time,
                           value.cycleTime, value.repeat, value.kind, value.id);
  }
  static void Restore(const ::Smp::ISimulator *simulator,
                      ::Smp::IStorageReader *reader,
                      ::Xsmp::Services::XsmpScheduler::Event &value) {
    ::Xsmp::Persist::Restore(
        simulator, reader, value.entryPoint, value.nextScheduleSimulationTime,
        value.time, value.cycleTime, value.repeat, value.kind, value.id);
  }
};
} // namespace Xsmp::Persist

namespace Xsmp::Services {
static constexpr ::Smp::Duration MaxDuration =
    std::numeric_limits<::Smp::Duration>::max();

XsmpScheduler::XsmpScheduler(::Smp::String8 name, ::Smp::String8 description,
                             ::Smp::IComposite *parent,
                             ::Smp::ISimulator *simulator)
    : XsmpSchedulerGen::XsmpSchedulerGen(name, description, parent, simulator) {

  // post an event to Hold the simulation at the maximal duration
  const ::Smp::Services::EventId holdId = -2;
  _events.try_emplace(
      holdId, Event{&HoldEvent, MaxDuration, MaxDuration, 0, 0,
                    ::Smp::Services::TimeKind::TK_SimulationTime, holdId});
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

  _zuluThread = std::thread(&XsmpScheduler::InternalZuluRun, this, simulator);
}

void XsmpScheduler::DoDisconnect() {
  // stop the zulu thread
  {
    std::scoped_lock lck{_zuluEventsTableMutex};
    if (_terminate) // if already terminated do nothing
      return;
    _terminate = true;
  }
  _zuluCv.notify_one();
  if (_zuluThread.joinable())
    _zuluThread.join();
}
void XsmpScheduler::SetTargetSpeed(double speed) {
  if (speed < 0.01)
    _targetSpeed = 0.01;
  else if (speed > 100.)
    _targetSpeed = 100.;
  else
    _targetSpeed = speed;
}

double XsmpScheduler::GetTargetSpeed() const noexcept { return _targetSpeed; }

::Smp::Services::EventId
XsmpScheduler::AddImmediateEvent(const ::Smp::IEntryPoint *entryPoint) {

  std::scoped_lock lck{_eventsMutex};
  ++_lastEventId;
  auto time = GetSimulator()->GetTimeKeeper()->GetSimulationTime();
  _events.try_emplace(_lastEventId,
                      Event{entryPoint, time, time, 0, 0,
                            ::Smp::Services::TimeKind::TK_SimulationTime,
                            _lastEventId});

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
      simulationTime < currentSimulationTime)
    ::Xsmp::Exception::throwInvalidEventTime(this, simulationTime,
                                             currentSimulationTime);

  if (repeat != 0 && cycleTime <= 0)
    ::Xsmp::Exception::throwInvalidCycleTime(this, cycleTime);

  std::scoped_lock lck{_eventsMutex};

  ++_lastEventId;
  _events.try_emplace(_lastEventId,
                      Event{entryPoint, simulationTime, time, cycleTime, repeat,
                            kind, _lastEventId});

  _events_table.try_emplace(simulationTime).first->second.emplace(_lastEventId);

  GetSimulator()->GetLogger()->Log(entryPoint, "Event posted",
                                   ::Smp::Services::ILogger::LMK_Debug);

  return _lastEventId;
}

::Smp::Services::EventId XsmpScheduler::AddSimulationTimeEvent(
    const ::Smp::IEntryPoint *entryPoint, ::Smp::Duration simulationTime,
    ::Smp::Duration cycleTime, ::Smp::Int64 repeat) {

  ::Smp::Duration time = GetSimulator()->GetTimeKeeper()->GetSimulationTime() +
                         simulationTime; // TODO check overflow
  return AddEvent(entryPoint, time, time, cycleTime, repeat,
                  ::Smp::Services::TimeKind::TK_SimulationTime);
}

::Smp::Services::EventId XsmpScheduler::AddMissionTimeEvent(
    const ::Smp::IEntryPoint *entryPoint, ::Smp::Duration missionTime,
    ::Smp::Duration cycleTime, ::Smp::Int64 repeat) {

  auto simulationTime =
      GetSimulator()->GetTimeKeeper()->GetSimulationTime() + missionTime -
      GetSimulator()->GetTimeKeeper()->GetMissionTime(); // TODO check overflow

  return AddEvent(entryPoint, simulationTime, missionTime, cycleTime, repeat,
                  ::Smp::Services::TimeKind::TK_MissionTime);
}

::Smp::Services::EventId XsmpScheduler::AddEpochTimeEvent(
    const ::Smp::IEntryPoint *entryPoint, ::Smp::DateTime epochTime,
    ::Smp::Duration cycleTime, ::Smp::Int64 repeat) {

  auto simulationTime =
      GetSimulator()->GetTimeKeeper()->GetSimulationTime() + epochTime -
      GetSimulator()->GetTimeKeeper()->GetEpochTime(); // TODO check overflow

  return AddEvent(entryPoint, simulationTime, epochTime, cycleTime, repeat,
                  ::Smp::Services::TimeKind::TK_EpochTime);
}

::Smp::Services::EventId XsmpScheduler::AddZuluTimeEvent(
    const ::Smp::IEntryPoint *entryPoint, ::Smp::DateTime zuluTime,
    ::Smp::Duration cycleTime, ::Smp::Int64 repeat) {

  // check zulu time not in the past
  if (auto currentZulu = GetSimulator()->GetTimeKeeper()->GetZuluTime();
      zuluTime < currentZulu)
    ::Xsmp::Exception::throwInvalidEventTime(this, zuluTime, currentZulu);

  if (repeat != 0 && cycleTime <= 0)
    ::Xsmp::Exception::throwInvalidCycleTime(this, cycleTime);

  Smp::Services::EventId eventId;
  {
    // create the event
    std::scoped_lock lck{_eventsMutex, _zuluEventsTableMutex};
    eventId = ++_lastEventId;
    _events.try_emplace(eventId,
                        Event{entryPoint, zuluTime, zuluTime, cycleTime, repeat,
                              ::Smp::Services::TimeKind::TK_ZuluTime, eventId});

    // insert the event in the zulu event table
    _zulu_events_table.try_emplace(zuluTime).first->second.emplace(eventId);
  }
  _zuluCv.notify_one();

  GetSimulator()->GetLogger()->Log(entryPoint, "Event posted",
                                   ::Smp::Services::ILogger::LMK_Debug);
  return eventId;
}

void XsmpScheduler::SetEventTime(::Smp::Services::EventId event,
                                 ::Smp::Duration simulationTime,
                                 ::Smp::Duration time,
                                 ::Smp::Services::TimeKind kind) {

  std::scoped_lock lck{_eventsMutex};
  auto it = _events.find(event);

  if (it == _events.end() || it->second.kind != kind)
    ::Xsmp::Exception::throwInvalidEventId(this, event);

  auto &events = _events_table[it->second.nextScheduleSimulationTime];

  events.erase(event);

  if (events.empty())
    _events_table.erase(it->second.nextScheduleSimulationTime);

  if (simulationTime < GetSimulator()->GetTimeKeeper()->GetSimulationTime()) {
    _events.erase(it);
    return;
  }

  it->second.nextScheduleSimulationTime = simulationTime;
  it->second.time = time;

  _events_table.try_emplace(simulationTime).first->second.emplace(event);
}

void XsmpScheduler::SetEventSimulationTime(::Smp::Services::EventId event,
                                           ::Smp::Duration simulationTime) {

  ::Smp::Duration time =
      GetSimulator()->GetTimeKeeper()->GetSimulationTime() + simulationTime;
  SetEventTime(event, time, time, ::Smp::Services::TimeKind::TK_SimulationTime);
}

void XsmpScheduler::SetEventMissionTime(::Smp::Services::EventId event,
                                        ::Smp::Duration missionTime) {
  ::Smp::Duration time = GetSimulator()->GetTimeKeeper()->GetSimulationTime() +
                         missionTime -
                         GetSimulator()->GetTimeKeeper()->GetMissionTime();
  SetEventTime(event, time, missionTime,
               ::Smp::Services::TimeKind::TK_MissionTime);
}

void XsmpScheduler::SetEventEpochTime(::Smp::Services::EventId event,
                                      ::Smp::DateTime epochTime) {
  ::Smp::Duration time = GetSimulator()->GetTimeKeeper()->GetSimulationTime() +
                         epochTime -
                         GetSimulator()->GetTimeKeeper()->GetEpochTime();
  SetEventTime(event, time, epochTime, ::Smp::Services::TimeKind::TK_EpochTime);
}

void XsmpScheduler::SetEventZuluTime(::Smp::Services::EventId event,
                                     ::Smp::DateTime zuluTime) {
  {
    auto currentZulu = GetSimulator()->GetTimeKeeper()->GetZuluTime();
    std::scoped_lock lck{_eventsMutex, _zuluEventsTableMutex};
    auto it = _events.find(event);

    if (it == _events.end() ||
        it->second.kind != ::Smp::Services::TimeKind::TK_ZuluTime)
      ::Xsmp::Exception::throwInvalidEventId(this, event);

    auto &events = _zulu_events_table[it->second.nextScheduleSimulationTime];

    events.erase(event);

    if (zuluTime < currentZulu) {
      _events.erase(it);
      // TODO log warning
      return;
    }

    it->second.nextScheduleSimulationTime = zuluTime;
    _zulu_events_table.try_emplace(zuluTime).first->second.emplace(event);
  }
  _zuluCv.notify_one();
}

void XsmpScheduler::SetEventCycleTime(::Smp::Services::EventId event,
                                      ::Smp::Duration cycleTime) {

  std::scoped_lock lck{_eventsMutex};
  auto it = _events.find(event);
  if (it == _events.end())
    ::Xsmp::Exception::throwInvalidEventId(this, event);

  if (it->second.repeat > 0 && cycleTime <= 0)
    ::Xsmp::Exception::throwInvalidCycleTime(this, cycleTime);

  it->second.cycleTime = cycleTime;
}

void XsmpScheduler::SetEventRepeat(::Smp::Services::EventId event,
                                   ::Smp::Int64 repeat) {

  std::scoped_lock lck{_eventsMutex};
  auto it = _events.find(event);

  if (it == _events.end())
    ::Xsmp::Exception::throwInvalidEventId(this, event);

  if (repeat != 0 && it->second.cycleTime <= 0)
    ::Xsmp::Exception::throwInvalidCycleTime(this, it->second.cycleTime);

  it->second.repeat = repeat;
}

void XsmpScheduler::RemoveEvent(::Smp::Services::EventId event) {

  std::scoped_lock lck{_eventsMutex};
  auto it = _events.find(event);
  if (it == _events.end())
    ::Xsmp::Exception::throwInvalidEventId(this, event);

  if (_currentEventId == event) {
    it->second.repeat = 0;
    return;
  }

  if (it->second.kind == ::Smp::Services::TimeKind::TK_ZuluTime) {
    std::scoped_lock lckZulu{_zuluEventsTableMutex};

    auto &events = _zulu_events_table[it->second.nextScheduleSimulationTime];

    events.erase(event);

    if (events.empty())
      _events_table.erase(it->second.nextScheduleSimulationTime);
  } else {

    auto &events = _events_table[it->second.nextScheduleSimulationTime];

    events.erase(event);

    if (events.empty())
      _events_table.erase(it->second.nextScheduleSimulationTime);
  }
  _events.erase(it);
}

::Smp::Services::EventId XsmpScheduler::GetCurrentEventId() const {
  std::scoped_lock lck{_eventsMutex};
  return _currentEventId;
}

::Smp::Duration XsmpScheduler::GetNextScheduledEventTime() const {
  std::scoped_lock lck{_eventsMutex};
  if (_events_table.begin() != _events_table.end())
    return _events_table.begin()->first;
  return std::numeric_limits<::Smp::Duration>::max();
}

void XsmpScheduler::Execute(::Smp::Services::EventId eventId) {

  std::unique_lock lck{_eventsMutex};

  auto &event = _events.at(eventId);

  // skip event if epoch/mission time has changed and event is in the past
  bool skip;
  switch (event.kind) {
  case ::Smp::Services::TimeKind::TK_EpochTime:
    skip = event.time < GetSimulator()->GetTimeKeeper()->GetEpochTime();
    break;
  case ::Smp::Services::TimeKind::TK_MissionTime:
    skip = event.time < GetSimulator()->GetTimeKeeper()->GetMissionTime();
    break;
  default:
    skip = false;
    break;
  }

  if (!skip) {
    lck.unlock();

    std::unique_lock lck2(_execMutex);
    _currentEventId = eventId;
    ::Xsmp::Helper::SafeExecute(GetSimulator(), event.entryPoint);
    _currentEventId = -1;
    lck2.unlock();
    lck.lock();
  }

  if (event.repeat == 0) {
    // remove the event
    _events.erase(_events.find(eventId));
  } else {
    // decrement the repeat
    if (event.repeat > 0)
      event.repeat--;
    // compute the next time the event will be executed
    event.nextScheduleSimulationTime += event.cycleTime;
    event.time += event.cycleTime;

    // post the event in the table
    _events_table.try_emplace(event.nextScheduleSimulationTime)
        .first->second.emplace(eventId);
  }
}

void XsmpScheduler::ExecuteZulu(::Smp::Services::EventId eventId) {

  std::unique_lock lck(_eventsMutex);

  auto &event = _events.at(eventId);

  lck.unlock();

  // execute the event only in executing and standby states
  if (auto state = GetSimulator()->GetState();
      state == ::Smp::SimulatorStateKind::SSK_Executing ||
      state == ::Smp::SimulatorStateKind::SSK_Standby) {

    std::scoped_lock lck2(_execMutex);
    _currentEventId = eventId;
    ::Xsmp::Helper::SafeExecute(GetSimulator(), event.entryPoint);
    _currentEventId = -1;
  }

  lck.lock();

  if (event.repeat == 0) {
    // remove the event
    _events.erase(_events.find(eventId));
  } else {
    // decrement the repeat
    if (event.repeat > 0)
      event.repeat--;
    // compute the next time the event will be executed
    event.nextScheduleSimulationTime += event.cycleTime;
    // post the event in the table
    std::scoped_lock lckZulu{_zuluEventsTableMutex};
    _zulu_events_table.try_emplace(event.nextScheduleSimulationTime)
        .first->second.emplace(eventId);
  }
}

bool XsmpScheduler::ExecuteEvents(EventList &events) {

  // execute all events
  while (!events.empty()) {
    // swap the current list of events
    EventList current;
    current.swap(events);
    for (auto it = current.begin(); it != current.end(); ++it) {
      Execute(*it);
      // process immediate events posted by this event
      if (_simulationStatus == Status::Hold || !ExecuteImmediateEvents()) {
        // store un-executed events and exit
        events.insert(it, current.end());
        return false;
      }
    }
  }
  return true;
}
bool XsmpScheduler::ExecuteImmediateEvents() {
  // execute all events
  while (!_immediate_events.empty()) {
    // swap  the current list of events
    EventList events;
    events.swap(_immediate_events);

    for (auto it = events.begin(); it != events.end(); ++it) {
      Execute(*it);
      // process immediate events posted by this event
      if (_simulationStatus == Status::Hold) {

        // store un-executed events and exit
        _immediate_events.insert(it, events.end());
        return false;
      }
    }
  }
  return true;
}

void XsmpScheduler::InternalZuluRun(const ::Smp::ISimulator *sim) {

  std::unique_lock lck(_zuluEventsTableMutex);

  while (!_terminate) {

    // execute all events with time <= current zulu time
    for (auto it = _zulu_events_table.begin();
         it != _zulu_events_table.end() &&
         it->first <= sim->GetTimeKeeper()->GetZuluTime();
         it = _zulu_events_table.erase(it)) {

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
          if (_terminate)
            return;
        }
      }
    }

    // wait until next event
    if (_zulu_events_table.empty())
      // exit if terminate signal received or a new event is posted.
      // in case a new event is posted, exit the loop and wait for the remaining
      // time (else branch)
      _zuluCv.wait(
          lck, [this]() { return _terminate || !_zulu_events_table.empty(); });
    else
      _zuluCv.wait_for(lck,
                       std::chrono::nanoseconds{
                           std::max(static_cast<::Smp::Duration>(0),
                                    _zulu_events_table.begin()->first -
                                        sim->GetTimeKeeper()->GetZuluTime())},
                       [this, sim] {
                         return _terminate ||
                                (!_zulu_events_table.empty() &&
                                 _zulu_events_table.begin()->first <=
                                     sim->GetTimeKeeper()->GetZuluTime());
                       });
  }
}

void XsmpScheduler::Restore(::Smp::IStorageReader *reader) {
  std::scoped_lock lck{_eventsMutex};
  ::Xsmp::Persist::Restore(GetSimulator(), this, reader, _events, _events_table,
                           _immediate_events, _lastEventId);
}

void XsmpScheduler::Store(::Smp::IStorageWriter *writer) {
  std::scoped_lock lck{_eventsMutex};
  ::Xsmp::Persist::Store(GetSimulator(), this, writer, _events, _events_table,
                         _immediate_events, _lastEventId);
}

void XsmpScheduler::_LeaveExecuting() {

  std::unique_lock lck{_holdMutex};
  _simulationStatus = Status::Hold;
  lck.unlock();
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
  if (!ExecuteImmediateEvents())
    return; // exit immediately in case of hold

  auto *eventManager = GetSimulator()->GetEventManager();
  // execute all events
  for (auto it = _events_table.begin(); it != _events_table.end();
       it = _events_table.erase(it)) {

    // notify that simulation time will be changed
    eventManager->Emit(::Smp::Services::IEventManager::SMP_PreSimTimeChangeId,
                       false);

    if (_simulationStatus == Status::Hold)
      return; // exit immediately if hold is requested

    auto duration = it->first - timeKeeper->GetSimulationTime();
    delay += static_cast<::Smp::Duration>(static_cast<double>(duration) /
                                          _targetSpeed) -
             (timeKeeper->GetZuluTime() - startZuluTime);

    auto endZuluTime = timeKeeper->GetZuluTime();
    // update speed
    if (duration)
      _speed.AddSample(static_cast<double>(endZuluTime - startZuluTime) /
                       static_cast<double>(duration));

    startZuluTime = endZuluTime;

    // TODO handle free running
    //  keep synchronized with zulu time
    if (delay > 0) {
      std::unique_lock lck{_holdMutex};
      if (_holdCv.wait_for(lck, std::chrono::nanoseconds{delay}, [this] {
            // continue to wait while Hold is not requested
            return _simulationStatus == Status::Hold;
          })) {
        return; // exit immediately in case of hold
      }
    }
    // change the simulation time
    timeKeeper->SetSimulationTime(it->first);

    // notify that simulation time has changed
    eventManager->Emit(::Smp::Services::IEventManager::SMP_PostSimTimeChangeId);

    // process all events (check for eventually events added by
    // SMP_PostSimTimeChangeId )
    if (!ExecuteImmediateEvents() || !ExecuteEvents(it->second))
      return; // exit immediately in case of hold
  }

  _load.clear();
  _speed.clear();
}

void XsmpScheduler::_HoldEvent() {
  // return in standby state
  GetSimulator()->Hold(true);
}

void XsmpScheduler::MovingAverage::AddSample(double sample) {
  std::scoped_lock lck{_mutex};
  sum = sum + sample - samples[index];
  samples[index] = sample;
  ++index;
  if (index == sampleCount)
    index = 0;

  if (size < sampleCount)
    ++size;
}
double XsmpScheduler::MovingAverage::GetAverage() {
  std::scoped_lock lck{_mutex};
  return size ? sum / size : 0.;
}
void XsmpScheduler::MovingAverage::clear() {
  std::scoped_lock lck{_mutex};
  size = 0;
  for (auto &sample : samples)
    sample = 0.;
  index = 0;
  sum = 0.;
}
} // namespace Xsmp::Services
