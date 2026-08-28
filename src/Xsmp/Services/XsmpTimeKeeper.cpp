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

#include <Smp/ISimulator.h>
#include <Smp/IStorageReader.h>
#include <Smp/IStorageWriter.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/IScheduler.h>
#include <Xsmp/DateTime.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Persist.h>
#include <Xsmp/Services/XsmpTimeKeeper.h>
#include <limits>
#include <mutex>

namespace Xsmp::Services {

void XsmpTimeKeeper::DoConnect(const ::Smp::ISimulator *simulator) const {

  simulator->GetEventManager()->Subscribe(
      ::Smp::Services::IEventManager::SMP_PreSimTimeChangeId,
      &PreSimTimeChange);

  simulator->GetEventManager()->Subscribe(
      ::Smp::Services::IEventManager::SMP_PostSimTimeChangeId,
      &PostSimTimeChange);
}
namespace {
/// The time bases are Int64 nanoseconds and are defined by their differences:
/// saturating keeps the result of a subtraction inside the type instead of
/// overflowing it.
::Smp::Duration subtract(::Smp::Int64 lhs, ::Smp::Int64 rhs) {
  constexpr auto highest = std::numeric_limits<::Smp::Int64>::max();
  constexpr auto lowest = std::numeric_limits<::Smp::Int64>::lowest();
  if (rhs < 0 && lhs > highest + rhs) {
    return highest;
  }
  if (rhs > 0 && lhs < lowest + rhs) {
    return lowest;
  }
  return lhs - rhs;
}
} // namespace

::Smp::Duration XsmpTimeKeeper::GetSimulationTime() const {
  return _simulationTime.read().get();
}

::Smp::DateTime XsmpTimeKeeper::GetMissionStartTime() const {
  return _missionStartTime.read().get();
}

::Smp::DateTime XsmpTimeKeeper::GetEpochTime() const {
  // the values are read one at a time: holding several locks at once would
  // take them in the order the compiler chooses to evaluate the operands
  const auto simulationTime = _simulationTime.read().get();
  const auto epochStart = _epochStart.read().get();
  return subtract(simulationTime, epochStart);
}

::Smp::Duration XsmpTimeKeeper::GetMissionTime() const {
  const auto simulationTime = _simulationTime.read().get();
  const auto epochStart = _epochStart.read().get();
  const auto missionStartTime = _missionStartTime.read().get();
  return subtract(subtract(simulationTime, epochStart), missionStartTime);
}

::Smp::DateTime XsmpTimeKeeper::GetZuluTime() const {
  return static_cast<::Smp::DateTime>(::Xsmp::DateTime::now());
}

void XsmpTimeKeeper::SetEpochTime(::Smp::DateTime epochTime) {
  const auto simulationTime = _simulationTime.read().get();
  _epochStart.write().get() = subtract(simulationTime, epochTime);
  GetSimulator()->GetEventManager()->Emit(
      ::Smp::Services::IEventManager::SMP_EpochTimeChangedId);
}

void XsmpTimeKeeper::SetMissionStartTime(::Smp::DateTime missionStart) {
  _missionStartTime.write().get() = missionStart;
  GetSimulator()->GetEventManager()->Emit(
      ::Smp::Services::IEventManager::SMP_MissionTimeChangedId);
}

void XsmpTimeKeeper::SetMissionTime(::Smp::Duration missionTime) {
  const auto simulationTime = _simulationTime.read().get();
  const auto epochStart = _epochStart.read().get();
  _missionStartTime.write().get() =
      subtract(subtract(simulationTime, epochStart), missionTime);

  GetSimulator()->GetEventManager()->Emit(
      ::Smp::Services::IEventManager::SMP_MissionTimeChangedId);
}

void XsmpTimeKeeper::SetSimulationTime(::Smp::Duration simulationTime) {

  if (!_simTimeChanging) {
    return;
  }
  auto max = GetSimulator()->GetScheduler()->GetNextScheduledEventTime();
  if (auto current = _simulationTime.read().get();
      simulationTime < current || simulationTime > max) {
    ::Xsmp::Exception::throwInvalidSimulationTime(this, current, simulationTime,
                                                  max);
  }
  _simulationTime.write().get() = simulationTime;
}

void XsmpTimeKeeper::Restore(::Smp::IStorageReader *reader) {
  ::Smp::Duration simulationTime{};
  ::Smp::DateTime missionStartTime{};
  ::Smp::DateTime epochStart{};
  ::Xsmp::Persist::Restore(GetSimulator(), this, reader, simulationTime,
                           missionStartTime, epochStart);
  _simulationTime.write().get() = simulationTime;
  _missionStartTime.write().get() = missionStartTime;
  _epochStart.write().get() = epochStart;
}

void XsmpTimeKeeper::Store(::Smp::IStorageWriter *writer) {
  const auto simulationTime = _simulationTime.read().get();
  const auto missionStartTime = _missionStartTime.read().get();
  const auto epochStart = _epochStart.read().get();
  ::Xsmp::Persist::Store(GetSimulator(), this, writer, simulationTime,
                         missionStartTime, epochStart);
}
void XsmpTimeKeeper::_PreSimTimeChange() { _simTimeChanging = true; }
void XsmpTimeKeeper::_PostSimTimeChange() { _simTimeChanging = false; }
} // namespace Xsmp::Services
