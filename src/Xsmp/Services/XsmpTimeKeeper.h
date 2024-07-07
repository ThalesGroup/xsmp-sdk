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

#ifndef XSMP_SERVICES_XSMPTIMEKEEPER_H_
#define XSMP_SERVICES_XSMPTIMEKEEPER_H_

#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Services/XsmpTimeKeeperGen.h>
#include <atomic>
#include <mutex>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------
// ----------------------------------------------------------------------------

namespace Xsmp::Services {

/// This class is thread safe: it is possible to get time (simulation, epoch,
/// mission and zulu) from any thread
class XsmpTimeKeeper final : public XsmpTimeKeeperGen {
public:
  // ------------------------------------------------------------------------------------
  // -------------------------- Constructors/Destructor
  // --------------------------
  // ------------------------------------------------------------------------------------

  using XsmpTimeKeeperGen::XsmpTimeKeeperGen;

  /// Virtual destructor to release memory.
  ~XsmpTimeKeeper() noexcept override = default;

  /// Return Simulation time.
  /// Simulation time is a relative time that starts at 0.
  /// @return  Current simulation time.
  ::Smp::Duration GetSimulationTime() const override;

  /// Return Epoch time.
  ///  Epoch time is an absolute time with a fixed offset to
  /// simulation time. Epoch time typically progresses with
  /// simulation time, but can be changed with SetEpochTime.
  /// @return  Current epoch time.
  ::Smp::DateTime GetEpochTime() const override;

  /// Get mission start time.
  /// @return  Mission start date and time.
  ::Smp::DateTime GetMissionStartTime() const override;

  /// Return Mission time.
  /// Mission time is a relative time with a fixed offset to
  /// simulation time. Mission time typically progresses with
  /// simulation time, but can be changed with the two methods
  /// SetMissionTime and SetMissionStart. Further, mission time is
  /// updated when changing epoch time with SetEpochTime.
  /// @return  Current mission time.
  ::Smp::Duration GetMissionTime() const override;

  /// Return Zulu time.
  /// Zulu time is a system dependent time and not related to
  /// simulation time. Zulu time is typically related to the system
  /// clock of the computer.
  /// @return  Current Zulu time.
  ::Smp::DateTime GetZuluTime() const override;

  /// Manually advance Simulation time.
  /// This method can only be called during a PreSimTimeChange event.
  /// When the Time Keeper updates simulation time in response to the
  /// Scheduler executing a new event, the update is performed in
  /// three steps.
  /// 1. First the PreSimTimeChange event is emitted.
  /// 2. Then the Simulation time is changed to the time of the event
  /// that is about to be executed.
  /// 3. Finally the PostSimTimeChange event is emitted.
  /// This method can only set SimTime between current time and the
  /// time that is about to the set by the procedure described above.
  /// Also, invoking this method will not trigger again the
  /// PreSimTimeChange or PostSimTimeChange events.
  /// @param   simulationTime New value of simulation time to set in
  ///          the Time Keeper. This has to be in the future.
  /// @throws  ::Smp::Services::InvalidSimulationTime
  void SetSimulationTime(::Smp::Duration simulationTime) override;

  /// Set Epoch time.
  ///
  ///  Changes the offset between simulation time and epoch time.
  /// Calling this method shall raise a global EpochTimeChanged event
  /// in the Event Manager.
  /// @param   epochTime New epoch time.
  void SetEpochTime(::Smp::DateTime epochTime) override;

  /// Set Mission time by defining the mission start time.
  /// In future calls to GetMissionTime, the mission time is
  /// calculated using the formula MissionTime = EpochTime -
  /// missionStart.
  /// Changes the offset between simulation time and mission time.
  /// The mission time itself will be calculated as the offset
  /// between the current epoch time and the given mission start:
  /// MissionTime = EpochTime - MissionStart
  ///
  /// Calling this method shall raise a global MissionTimeChanged
  /// event in the Event Manager.
  /// @param   missionStart New mission start date and time.
  void SetMissionStartTime(::Smp::DateTime missionStart) override;

  /// Set Mission time by providing the current mission time.
  /// This effectively sets the MissionStartTime using the formula
  /// MissionStartTime = EpochTime - missionTime.
  /// Changes the offset between simulation time and mission time.
  /// Calling this method shall raise a global MissionTimeChanged
  /// event in the Event Manager.
  /// @param   missionTime New mission time.
  void SetMissionTime(::Smp::Duration missionTime) override;

  void Restore(::Smp::IStorageReader *reader) override;

  void Store(::Smp::IStorageWriter *writer) override;
  void _PreSimTimeChange() override;
  void _PostSimTimeChange() override;

private:
  friend class ::Xsmp::Component::Helper;
  mutable std::mutex _mutex;
  ::Smp::Duration _simulationTime{};
  ::Smp::DateTime _missionStartTime{};
  ::Smp::DateTime _epochStart{};
  std::atomic_bool _simTimeChanging{};
  void DoConnect(const ::Smp::ISimulator *simulator) const;
};
} // namespace Xsmp::Services

#endif // XSMP_SERVICES_XSMPTIMEKEEPER_H_
