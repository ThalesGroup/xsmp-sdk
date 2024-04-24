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

#ifndef XSMP_SERVICES_XSMPSCHEDULER_H_
#define XSMP_SERVICES_XSMPSCHEDULER_H_

#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/EventId.h>
#include <Smp/Services/TimeKind.h>
#include <Xsmp/Persist.h>
#include <Xsmp/Services/XsmpSchedulerGen.h>
#include <atomic>
#include <condition_variable>
#include <map>
#include <mutex>
#include <set>
#include <thread>
#include <type_traits>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------
// ----------------------------------------------------------------------------

namespace Xsmp::Services {

class XsmpScheduler final : public XsmpSchedulerGen {
public:
  // ------------------------------------------------------------------------------------
  // -------------------------- Constructors/Destructor
  // --------------------------
  // ------------------------------------------------------------------------------------

  /// Constructor setting name, description and parent.
  /// @param name Name of new model instance.
  /// @param description Description of new model instance.
  /// @param parent Parent of new model instance.
  XsmpScheduler(::Smp::String8 name, ::Smp::String8 description,
                ::Smp::IComposite *parent, ::Smp::ISimulator *simulator);

  /// Virtual destructor to release memory.
  ~XsmpScheduler() override;

  /// Add an immediate event to the scheduler.
  /// An immediate event is an event that will be added as a
  /// simulation time event (with simulation delta time of 0) at the
  /// front of the event queue. As such, it will be executed when the
  /// scheduler processes its simulation time events again, but not
  /// immediately in the call to AddImmediateEvent().
  /// When the simulator is in Standby state, simulation time does
  /// not progress, and simulation time events (including immediate
  /// events) are not processed.
  /// @remarks To execute an entry point immediately without going
  ///          through the scheduler, its Execute() method can be
  ///          called.
  /// @remarks For events scheduled at the same time (including
  ///          immediate events), the Scheduler policy is to always
  ///          execute those first that have been posted first
  ///          ("First scheduled, first executed").
  /// @param   entryPoint Entry point to call from event.
  /// @return  Event identifier that can be used to change or remove
  ///          event.
  ::Smp::Services::EventId
  AddImmediateEvent(const ::Smp::IEntryPoint *entryPoint) override;

  /// Add event to scheduler that is called based on simulation time.
  /// An event with repeat=0 is not cyclic. It will be removed
  /// automatically after is has been triggered.
  /// An event with repeat>0 is cyclic, and will be repeated 'repeat'
  /// times. Therefore, it will be called repeat+1 times, and then it
  /// will be removed automatically.
  /// An event with repeat=-1 is cyclic as well, but it will be
  /// triggered forever, unless it is removed from the scheduler
  /// using the RemoveEvent() method.
  /// For a cyclic event, the cycleTime needs to be positive.
  /// Otherwise, an InvalidCycleTime exception is thrown. For
  /// non-cyclic events, it is stored but not used.
  /// The simulationTime must not be negative. Otherwise, an
  /// InvalidEventTime exception is thrown.
  /// @remarks For events scheduled at the same time (including
  ///          immediate events), the Scheduler policy is to always
  ///          execute those first that have been posted first
  ///          ("First scheduled, first executed").
  /// @param   entryPoint Entry point to call from event.
  /// @param   simulationTime Duration from now when to trigger the
  ///          event for the first time.
  ///          This must not be a negative simulation time, as this
  ///          would be in the past.
  /// @param   cycleTime Duration between two triggers of the event.
  /// When repeat is not zero, this must be a positive duration.
  /// @param   repeat Number of times the event shall be repeated, or
  ///          0 for a single event, or -1 for no limit.
  /// @return  Event identifier that can be used to change or remove
  ///          event.
  /// @throws  ::Smp::Services::InvalidCycleTime
  /// @throws  ::Smp::Services::InvalidEventTime
  ::Smp::Services::EventId AddSimulationTimeEvent(
      const ::Smp::IEntryPoint *entryPoint, ::Smp::Duration simulationTime,
      ::Smp::Duration cycleTime = 0, ::Smp::Int64 repeat = 0) override;

  /// Add event to scheduler that is called based on mission time.
  /// An event with repeat=0 is not cyclic. It will be removed
  /// automatically after is has been triggered.
  /// An event with repeat>0 is cyclic, and will be repeated repeat
  /// times. Therefore, it will be called repeat+1 times, and then it
  /// will be removed automatically.
  /// An event with repeat=-1 is cyclic as well, but it will be
  /// triggered forever, unless it is removed from the scheduler
  /// using the RemoveEvent() method.
  /// For a cyclic event, the cycleTime needs to be positive.
  /// Otherwise, an InvalidCycleTime exception is thrown. For
  /// non-cyclic events, it is stored but not used.
  /// The missionTime must not be before the current mission time of
  /// the ITimeKeeper service. Otherwise, an InvalidEventTime
  /// exception is thrown.
  /// @remarks For events scheduled at the same time (including
  ///          immediate events), the Scheduler policy is to always
  ///          execute those first that have been posted first
  ///          ("First scheduled, first executed").
  /// @param   entryPoint Entry point to call from event.
  /// @param   missionTime Absolute mission time when to trigger the
  ///          event for the first time.
  /// This must not be a mission time in the past.
  /// @param   cycleTime Duration between two triggers of the event.
  /// When repeat is not zero, this must be a positive duration.
  /// @param   repeat Number of times the event shall be repeated, or
  ///          0 for a single event, or -1 for no limit.
  /// @return  Event identifier that can be used to change or remove
  ///          event.
  /// @throws  ::Smp::Services::InvalidCycleTime
  /// @throws  ::Smp::Services::InvalidEventTime
  ::Smp::Services::EventId AddMissionTimeEvent(
      const ::Smp::IEntryPoint *entryPoint, ::Smp::Duration missionTime,
      ::Smp::Duration cycleTime = 0, ::Smp::Int64 repeat = 0) override;

  /// Add event to scheduler that is called based on epoch time.
  /// An event with repeat=0 is not cyclic. It will be removed
  /// automatically after is has been triggered.
  /// An event with repeat>0 is cyclic, and will be repeated repeat
  /// times. Therefore, it will be called repeat+1 times, and then it
  /// will be removed automatically.
  /// An event with repeat=-1 is cyclic as well, but it will be
  /// triggered forever, unless it is removed from the scheduler
  /// using the RemoveEvent() method.
  /// For a cyclic event, the cycleTime needs to be positive.
  /// Otherwise, an InvalidCycleTime exception is thrown. For
  /// non-cyclic events, it is stored but not used.
  /// The epochTime must not be before the current epoch time of the
  /// ITimeKeeper service. Otherwise, an InvalidEventTime exception
  /// is thrown.
  /// @remarks For events scheduled at the same time (including
  ///          immediate events), the Scheduler policy is to always
  ///          execute those first that have been posted first
  ///          ("First scheduled, first executed").
  /// @param   entryPoint Entry point to call from event.
  /// @param   epochTime Epoch time when to trigger the event for the
  ///          first time.
  /// This must not be an epoch time in the past.
  /// @param   cycleTime Duration between two triggers of the event.
  /// When repeat is not zero, this must be a positive duration.
  /// @param   repeat Number of times the event shall be repeated, or
  ///          0 for a single event, or -1 for no limit.
  /// @return  Event identifier that can be used to change or remove
  ///          event.
  /// @throws  ::Smp::Services::InvalidCycleTime
  /// @throws  ::Smp::Services::InvalidEventTime
  ::Smp::Services::EventId
  AddEpochTimeEvent(const ::Smp::IEntryPoint *entryPoint,
                    ::Smp::DateTime epochTime, ::Smp::Duration cycleTime = 0,
                    ::Smp::Int64 repeat = 0) override;

  /// Add event to scheduler that is called based on Zulu time.
  /// An event with repeat=0 is not cyclic. It will be removed
  /// automatically after is has been triggered.
  /// An event with repeat>0 is cyclic, and will be repeated repeat
  /// times. Therefore, it will be called repeat+1 times, and then it
  /// will be removed automatically.
  /// An event with repeat=-1 is cyclic as well, but it will be
  /// triggered forever, unless it is removed from the scheduler
  /// using the RemoveEvent() method.
  /// For a cyclic event, the cycleTime needs to be positive.
  /// Otherwise, an InvalidCycleTime exception is thrown. For
  /// non-cyclic events, it is stored but not used.
  /// The zuluTime must not be before the current Zulu time of the
  /// ITimeKeeper service. Otherwise, an InvalidEventTime exception
  /// is thrown.
  /// @remarks For events scheduled at the same time (including
  ///          immediate events), the Scheduler policy is to always
  ///          execute those first that have been posted first
  ///          ("First scheduled, first executed").
  /// @param   entryPoint Entry point to call from event.
  /// @param   zuluTime Absolute (Zulu) time when to trigger the
  ///          event for the first time.
  /// This must not be a time in the past.
  /// @param   cycleTime Duration between two triggers of the event.
  /// When repeat is not zero, this must be a positive duration.
  /// @param   repeat Number of times the event shall be repeated, or
  ///          0 for a single event, or -1 for no limit.
  /// @return  Event identifier that can be used to change or remove
  ///          event.
  /// @throws  ::Smp::Services::InvalidCycleTime
  /// @throws  ::Smp::Services::InvalidEventTime
  ::Smp::Services::EventId
  AddZuluTimeEvent(const ::Smp::IEntryPoint *entryPoint,
                   ::Smp::DateTime zuluTime, ::Smp::Duration cycleTime = 0,
                   ::Smp::Int64 repeat = 0) override;

  /// Update when an existing simulation time event on the scheduler
  /// shall be triggered.
  /// When the given event Id is not a valid identifier of a
  /// scheduler event, the method throws an exception of type
  /// InvalidEventId. In case an event is registered under the given
  /// event Id but it is not a simulation time event, the method
  /// throws an exception of type InvalidEventId as well.
  ///
  /// The simulationTime must not be negative. Otherwise, the event
  /// will never be executed, but immediately removed.
  /// @param   event Identifier of event to modify.
  /// @param   simulationTime Duration from now when to trigger event.
  /// If the simulation time is negative, the event will never be
  /// executed but instead be removed immediately from the scheduler.
  /// @throws  ::Smp::Services::InvalidEventId
  void SetEventSimulationTime(::Smp::Services::EventId event,
                              ::Smp::Duration simulationTime) override;

  /// Update when an existing mission time event on the scheduler
  /// shall be triggered.
  /// When the given event Id is not a valid identifier of a
  /// scheduler event, the method throws an exception of type
  /// InvalidEventId. In case an event is registered under the given
  /// event Id but it is not a mission time event, the method throws
  /// an exception of type InvalidEventId as well.
  ///
  /// The missionTime must not be before the current mission time of
  /// the ITimeKeeper service. Otherwise, the event will never be
  /// executed, but immediately removed.
  /// @param   event Identifier of event to modify.
  /// @param   missionTime Absolute mission time when to trigger event.
  /// If the mission time is before the current mission time, the
  /// event will never be executed but instead be removed immediately
  /// from the scheduler.
  /// @throws  ::Smp::Services::InvalidEventId
  void SetEventMissionTime(::Smp::Services::EventId event,
                           ::Smp::Duration missionTime) override;

  /// Update when an existing epoch time event on the scheduler (an
  /// event that has been registered using AddEpochTimeEvent()) shall
  /// be triggered.
  /// When the given event Id  is not a valid identifier of a
  /// scheduler event, the method throws an exception of type
  /// InvalidEventId. In case an event is registered under the given
  /// event Id but it is not an epoch time event, the method throws
  /// an exception of type InvalidEventId as well.
  ///
  /// The epochTime must not be before the current epoch time of the
  /// ITimeKeeper service. Otherwise, the event will never be
  /// executed, but immediately removed.
  /// @param   event Identifier of event to modify.
  /// @param   epochTime Epoch time when to trigger event.
  /// If the epoch time is before the current epoch time, the event
  /// will never be executed but instead be removed immediately from
  /// the scheduler.
  /// @throws  ::Smp::Services::InvalidEventId
  void SetEventEpochTime(::Smp::Services::EventId event,
                         ::Smp::DateTime epochTime) override;

  /// Update when an existing zulu time event on the scheduler shall
  /// be triggered.
  /// When the given event Id is not a valid identifier of a
  /// scheduler event, the method throws an exception of type
  /// InvalidEventId. In case an event is registered under the given
  /// event Id but it is not a zulu time event, the method throws an
  /// exception of type InvalidEventId as well.
  /// The zuluTime must not be before the current Zulu time of the
  /// ITimeKeeper service. Otherwise, the event will never be
  /// executed, but immediately removed.
  /// @param   event Identifier of event to modify.
  /// @param   zuluTime Absolute (Zulu) time when to trigger event.
  /// If the zulu time is before the current zulu time, the event
  /// will never be executed but instead be removed immediately from
  /// the scheduler.
  /// @throws  ::Smp::Services::InvalidEventId
  void SetEventZuluTime(::Smp::Services::EventId event,
                        ::Smp::DateTime zuluTime) override;

  /// Update cycle time of an existing event on the scheduler.
  /// When the given event is not a valid identifier of a scheduler
  /// event, the method throws an exception of type InvalidEventId.
  /// For a cyclic event, the cycleTime needs to be positive. For
  /// non-cyclic events, it is stored but not used.
  /// @param   event Identifier of event to modify.
  /// @param   cycleTime Duration between two triggers of the event.
  /// For a cyclic event, this needs to be a positive duration.
  /// Otherwise, an exception of type InvalidCycleTime is thrown.
  /// @throws  ::Smp::Services::InvalidCycleTime
  /// @throws  ::Smp::Services::InvalidEventId
  void SetEventCycleTime(::Smp::Services::EventId event,
                         ::Smp::Duration cycleTime) override;

  /// Update the repeat counter of an existing event on the
  /// scheduler.
  /// When the given event is not a valid identifier of a scheduler
  /// event, the method throws an exception of type InvalidEventId.
  /// An event with repeat=0 is not cyclic. It will be removed
  /// automatically after is has been triggered.
  /// An event with repeat>0 is cyclic, and will be repeated 'repeat'
  /// times. Therefore, it will be called repeat+1 times, and then it
  /// will be removed automatically.
  /// An event with repeat=-1 is cyclic as well, but it will be
  /// triggered forever, unless it is removed from the scheduler
  /// using the RemoveEvent() method.
  /// For a cyclic event, the cycleTime needs to be positive. For
  /// non-cyclic events, it is ignored.
  /// @param   event Identifier of event to modify.
  /// @param   repeat Number of times the event shall be repeated, or
  ///          0 for a single event, or -1 for no limit.
  ///          An event with a repeat different from 0 is called
  ///          cyclic. For such an event, a positive cycle time has
  ///          to be defined before. Otherwise, an exception of type
  ///          InvalidCycleTime is thrown.
  /// @throws  ::Smp::Services::InvalidEventId
  void SetEventRepeat(::Smp::Services::EventId event,
                      ::Smp::Int64 repeat) override;

  /// Remove an event from the scheduler.
  /// When the given event is not a valid identifier of a scheduler
  /// event, the method throws an exception of type InvalidEventId.
  /// An event with repeat=0 is removed automatically after it has
  /// been triggered.
  /// @param   event Event identifier of the event to remove.
  /// @throws  ::Smp::Services::InvalidEventId
  void RemoveEvent(::Smp::Services::EventId event) override;

  /// Return the ID of the event currently executed by the scheduler.
  /// If no event is executed, this function returns -1.
  /// The operation returns -1 when no event is being executed.
  /// @return  Event Id or -1 if no event is being executed.
  ::Smp::Services::EventId GetCurrentEventId() const override;

  /// Get the time of next scheduled Event.
  /// This function can only provide the time of the next event at
  /// the time of invocation. There is no guarantee that no events
  /// will be inserted between the method invocation and the
  /// (currently) next event.
  /// Events scheduled in Zulu Time are not considered, as these
  /// Events do not have a fixed defined Simulation Time.
  /// @return  Time of the next event on the scheduler.
  ::Smp::Duration GetNextScheduledEventTime() const override;

  void Restore(::Smp::IStorageReader *reader) override;

  void Store(::Smp::IStorageWriter *writer) override;

  void _HoldEvent() override;

  /// hold the scheduler
  void _LeaveExecuting() override;

  void _EnterExecuting() override;

  void SetTargetSpeed(double speed);
  double GetTargetSpeed() const noexcept;

private:
  friend class ::Xsmp::Component::Helper;

  struct Event {
    const ::Smp::IEntryPoint *entryPoint;
    ::Smp::Duration nextScheduleSimulationTime;
    ::Smp::Duration time;
    ::Smp::Duration cycleTime;
    ::Smp::Int64 repeat;
    ::Smp::Services::TimeKind kind;
    ::Smp::Services::EventId id;
  };

  friend struct ::Xsmp::Persist::Helper<Event>;

  // An EventList that is ordered by posting order
  // It uses a set of unsigned ::Smp::Services::EventId
  // to be sure that the first posted event is the first executed
  using EventList =
      std::set<::Smp::Services::EventId,
               std::less<std::make_unsigned_t<::Smp::Services::EventId>>>;

  // scheduling table for zulu time events
  mutable std::mutex _zuluEventsTableMutex{};
  std::map<::Smp::Duration, EventList> _zulu_events_table{};

  // Mapping between an EventId and internal data
  mutable std::mutex
      _eventsMutex{}; // protection for _events, _immediate_events,
                      // _events_table and _lastEventId
  // scheduling table for simu/epoch/mission time events
  std::map<::Smp::Duration, EventList> _events_table{};

  // immediate events  table
  EventList _immediate_events{};
  std::map<::Smp::Services::EventId, Event> _events{};
  ::Smp::Services::EventId _lastEventId = -1;

  // The current EventId that is being executed
  ::Smp::Services::EventId _currentEventId{-1};
  std::condition_variable _zuluCv{};

  std::condition_variable _holdCv{};
  std::mutex _holdMutex{};

  mutable std::mutex _execMutex{};
  enum class Status { Running, Hold };
  std::atomic<double> _targetSpeed{100.};
  std::atomic<Status> _simulationStatus{};
  bool _terminate{false};

  // thread that process zulu events
  std::thread _zuluThread{};

  class MovingAverage {
  public:
    void AddSample(double sample);
    double GetAverage();
    void clear();

  private:
    double sum{};
    static constexpr unsigned int sampleCount = 20;
    std::array<double, sampleCount> samples{};
    unsigned int index{};
    unsigned int size{};
    std::mutex _mutex{};
  };

  MovingAverage _load{};
  MovingAverage _speed{};

  /// Run the scheduler
  void InternalZuluRun(const ::Smp::ISimulator *simulator);

  ::Smp::Services::EventId
  AddEvent(const ::Smp::IEntryPoint *entryPoint, ::Smp::Duration simulationTime,
           ::Smp::Duration time, ::Smp::Duration cycleTime, ::Smp::Int64 repeat,
           ::Smp::Services::TimeKind kind);

  void SetEventTime(::Smp::Services::EventId id, ::Smp::Duration simulationTime,
                    ::Smp::Duration time, ::Smp::Services::TimeKind kind);

  /// Execute a specific event
  /// @param eventId the event id to execute
  void Execute(::Smp::Services::EventId eventId);

  /// Execute a specific event
  /// @param eventId the event id to execute
  void ExecuteZulu(::Smp::Services::EventId eventId);

  /// Execute a list of events
  /// @param events the vector of events to execute
  /// @return true if all events have been executed, false if hold immediate is
  /// requested and we have to exit
  bool ExecuteEvents(EventList &events);

  /// Execute all immediate events
  /// @return true if all events have been executed, false if hold immediate is
  /// requested and we have to exit
  bool ExecuteImmediateEvents();

  void DoConnect(const ::Smp::ISimulator *simulator);
  void DoDisconnect();
};

} // namespace Xsmp::Services

#endif // XSMP_SERVICES_XSMPSCHEDULER_H_
