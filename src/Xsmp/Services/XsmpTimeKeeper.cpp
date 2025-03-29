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
::Smp::Duration XsmpTimeKeeper::GetSimulationTime() const {
  return _simulationTime.read().get();
}

::Smp::DateTime XsmpTimeKeeper::GetMissionStartTime() const {
  return _missionStartTime.read().get();
}

::Smp::DateTime XsmpTimeKeeper::GetEpochTime() const {
  return _simulationTime.read().get() - _epochStart.read().get();
}

::Smp::Duration XsmpTimeKeeper::GetMissionTime() const {
  return _simulationTime.read().get() - _epochStart.read().get() -
         _missionStartTime.read().get();
}

::Smp::DateTime XsmpTimeKeeper::GetZuluTime() const {
  return static_cast<::Smp::DateTime>(::Xsmp::DateTime::now());
}

void XsmpTimeKeeper::SetEpochTime(::Smp::DateTime epochTime) {
  _epochStart.write().get() = _simulationTime.read().get() - epochTime;
  GetSimulator()->GetEventManager()->Emit(
      ::Smp::Services::IEventManager::SMP_EpochTimeChangedId);
}

void XsmpTimeKeeper::SetMissionStartTime(::Smp::DateTime missionStart) {
  _missionStartTime.write().get() = missionStart;
  GetSimulator()->GetEventManager()->Emit(
      ::Smp::Services::IEventManager::SMP_MissionTimeChangedId);
}

void XsmpTimeKeeper::SetMissionTime(::Smp::Duration missionTime) {
  _missionStartTime.write().get() =
      _simulationTime.read().get() - _epochStart.read().get() - missionTime;

  GetSimulator()->GetEventManager()->Emit(
      ::Smp::Services::IEventManager::SMP_MissionTimeChangedId);
}

void XsmpTimeKeeper::SetSimulationTime(::Smp::Duration simulationTime) {

  if (!_simTimeChanging) {
    return;
  }
  auto max = GetSimulator()->GetScheduler()->GetNextScheduledEventTime();
  auto current = _simulationTime.read().get();
  if (simulationTime < current || simulationTime > max) {
    ::Xsmp::Exception::throwInvalidSimulationTime(this, current, simulationTime,
                                                  max);
  }
  _simulationTime.write().get() = simulationTime;
}

void XsmpTimeKeeper::Restore(::Smp::IStorageReader *reader) {
  ::Xsmp::Persist::Restore(
      GetSimulator(), this, reader, _simulationTime.write().get(),
      _missionStartTime.write().get(), _epochStart.write().get());
}

void XsmpTimeKeeper::Store(::Smp::IStorageWriter *writer) {
  ::Xsmp::Persist::Store(
      GetSimulator(), this, writer, _simulationTime.read().get(),
      _missionStartTime.read().get(), _epochStart.read().get());
}
void XsmpTimeKeeper::_PreSimTimeChange() { _simTimeChanging = true; }
void XsmpTimeKeeper::_PostSimTimeChange() { _simTimeChanging = false; }
} // namespace Xsmp::Services
