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
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/IScheduler.h>
#include <Xsmp/DateTime.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Persist.h>
#include <Xsmp/Persist/StdAtomic.h>
#include <Xsmp/Services/XsmpTimeKeeper.h>
#include <Xsmp/Helper.h>

namespace Xsmp::Services {

void XsmpTimeKeeper::DoConnect(const ::Smp::ISimulator *simulator) const {

    simulator->GetEventManager()->Subscribe(
            ::Smp::Services::IEventManager::SMP_PreSimTimeChangeId,
            PreSimTimeChange);

    simulator->GetEventManager()->Subscribe(
            ::Smp::Services::IEventManager::SMP_PostSimTimeChangeId,
            PostSimTimeChange);
}
::Smp::Duration XsmpTimeKeeper::GetSimulationTime() const {
    std::scoped_lock lck { _mutex };
    return _simulationTime;
}

::Smp::DateTime XsmpTimeKeeper::GetMissionStartTime() const {
    std::scoped_lock lck { _mutex };
    return _missionStartTime;
}

::Smp::DateTime XsmpTimeKeeper::GetEpochTime() const {
    std::scoped_lock lck { _mutex };
    return _simulationTime - _epochStart;
}

::Smp::Duration XsmpTimeKeeper::GetMissionTime() const {
    std::scoped_lock lck { _mutex };
    return _simulationTime - _epochStart - _missionStartTime;
}

::Smp::DateTime XsmpTimeKeeper::GetZuluTime() const {
    return static_cast<::Smp::DateTime>(::Xsmp::DateTime::now());
}

void XsmpTimeKeeper::SetEpochTime(::Smp::DateTime epochTime) {

    std::unique_lock lck { _mutex };
    _epochStart = _simulationTime - epochTime;
    lck.unlock();

    GetSimulator()->GetEventManager()->Emit(
            ::Smp::Services::IEventManager::SMP_EpochTimeChangedId);
}

void XsmpTimeKeeper::SetMissionStartTime(::Smp::DateTime missionStart) {

    std::unique_lock lck { _mutex };
    _missionStartTime = missionStart;
    lck.unlock();
    GetSimulator()->GetEventManager()->Emit(
            ::Smp::Services::IEventManager::SMP_MissionTimeChangedId);
}

void XsmpTimeKeeper::SetMissionTime(::Smp::Duration missionTime) {

    std::unique_lock lck { _mutex };
    _missionStartTime = _simulationTime - _epochStart - missionTime;
    lck.unlock();
    GetSimulator()->GetEventManager()->Emit(
            ::Smp::Services::IEventManager::SMP_MissionTimeChangedId);
}

void XsmpTimeKeeper::SetSimulationTime(::Smp::Duration simulationTime) {

    if (!_simTimeChanging)
        return;

    auto max = GetSimulator()->GetScheduler()->GetNextScheduledEventTime();
    std::scoped_lock lck { _mutex };
    if (simulationTime < _simulationTime || simulationTime > max)
        ::Xsmp::Exception::throwInvalidSimulationTime(this, _simulationTime,
                simulationTime, max);

    _simulationTime = simulationTime;
}

void XsmpTimeKeeper::Restore(::Smp::IStorageReader *reader) {
    ::Xsmp::Persist::Restore(GetSimulator(), this, reader, _simulationTime,
            _missionStartTime, _epochStart);
}

void XsmpTimeKeeper::Store(::Smp::IStorageWriter *writer) {
    ::Xsmp::Persist::Store(GetSimulator(), this, writer, _simulationTime,
            _missionStartTime, _epochStart);
}
void XsmpTimeKeeper::_PreSimTimeChange() {
    _simTimeChanging = true;
}
void XsmpTimeKeeper::_PostSimTimeChange() {
    _simTimeChanging = false;
}
} // namespace Xsmp::Services
