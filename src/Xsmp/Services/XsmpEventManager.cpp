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
#include <Smp/Services/ILogger.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Persist.h>
#include <Xsmp/Persist/StdUnorderedMap.h>
#include <Xsmp/Persist/StdString.h>
#include <Xsmp/Persist/StdVector.h>
#include <Xsmp/Persist/SmpIObject.h>
#include <Xsmp/Services/XsmpEventManager.h>
#include <Xsmp/Helper.h>
#include <algorithm>

namespace Xsmp::Services {

using namespace ::Smp::Services;
XsmpEventManager::XsmpEventManager(::Smp::String8 name,
        ::Smp::String8 description, ::Smp::IObject *parent,
        ::Smp::ISimulator *simulator) :
        XsmpEventManagerGen::XsmpEventManagerGen(name, description, parent,
                simulator) {

    _events.try_emplace(IEventManager::SMP_LeaveConnecting,
            IEventManager::SMP_LeaveConnectingId);
    _events.try_emplace(IEventManager::SMP_EnterInitialising,
            IEventManager::SMP_EnterInitialisingId);
    _events.try_emplace(IEventManager::SMP_LeaveInitialising,
            IEventManager::SMP_LeaveInitialisingId);
    _events.try_emplace(IEventManager::SMP_EnterStandby,
            IEventManager::SMP_EnterStandbyId);
    _events.try_emplace(IEventManager::SMP_LeaveStandby,
            IEventManager::SMP_LeaveStandbyId);
    _events.try_emplace(IEventManager::SMP_EnterExecuting,
            IEventManager::SMP_EnterExecutingId);
    _events.try_emplace(IEventManager::SMP_LeaveExecuting,
            IEventManager::SMP_LeaveExecutingId);
    _events.try_emplace(IEventManager::SMP_EnterStoring,
            IEventManager::SMP_EnterStoringId);
    _events.try_emplace(IEventManager::SMP_LeaveStoring,
            IEventManager::SMP_LeaveStoringId);
    _events.try_emplace(IEventManager::SMP_EnterRestoring,
            IEventManager::SMP_EnterRestoringId);
    _events.try_emplace(IEventManager::SMP_LeaveRestoring,
            IEventManager::SMP_LeaveRestoringId);
    _events.try_emplace(IEventManager::SMP_EnterExiting,
            IEventManager::SMP_EnterExitingId);
    _events.try_emplace(IEventManager::SMP_EnterAborting,
            IEventManager::SMP_EnterAbortingId);
    _events.try_emplace(IEventManager::SMP_EpochTimeChanged,
            IEventManager::SMP_EpochTimeChangedId);
    _events.try_emplace(IEventManager::SMP_MissionTimeChanged,
            IEventManager::SMP_MissionTimeChangedId);
    _events.try_emplace(IEventManager::SMP_EnterReconnecting,
            IEventManager::SMP_EnterReconnectingId);
    _events.try_emplace(IEventManager::SMP_LeaveReconnecting,
            IEventManager::SMP_LeaveReconnectingId);
    _events.try_emplace(IEventManager::SMP_PreSimTimeChange,
            IEventManager::SMP_PreSimTimeChangeId);
    _events.try_emplace(IEventManager::SMP_PostSimTimeChange,
            IEventManager::SMP_PostSimTimeChangeId);

    for (const auto& [eventName, id] : _events)
        _ids.try_emplace(id, eventName);
}

::Smp::Services::EventId XsmpEventManager::QueryEventId(
        ::Smp::String8 eventName) {
    if (!eventName || eventName[0] == '\0')
        ::Xsmp::Exception::throwInvalidEventName(this, eventName);

    std::scoped_lock lck { _eventsMutex };

    if (auto it = _events.find(eventName); it != _events.end()) {
        return it->second;
    }

    auto eventId = static_cast<::Smp::Services::EventId>(_events.size() + 1);

    auto &name = _events.try_emplace(eventName, eventId).first->first;

    std::scoped_lock lck2 { _idsMutex };
    _ids.try_emplace(eventId, name);

    return eventId;
}

const std::string& XsmpEventManager::GetEventName(
        ::Smp::Services::EventId event) const {
    std::scoped_lock lck { _idsMutex };
    auto it = _ids.find(event);
    if (it == _ids.end())
        ::Xsmp::Exception::throwInvalidEventId(this, event);
    return it->second;
}

void XsmpEventManager::Subscribe(::Smp::Services::EventId event,
        const ::Smp::IEntryPoint *entryPoint) {

    auto event_name = GetEventName(event);

    std::unique_lock lck { _subscriptionsMutex };

    if (auto it = _subscriptions.find(event); it != _subscriptions.end()) {
        auto &entryPoints = it->second;

        if (auto it2 = std::find(entryPoints.begin(), entryPoints.end(),
                entryPoint); it2 != entryPoints.end())
            ::Xsmp::Exception::throwEntryPointAlreadySubscribed(this,
                    entryPoint, event_name);

        entryPoints.push_back(entryPoint);
    }
    else {

        _subscriptions.try_emplace(event,
                std::vector<const ::Smp::IEntryPoint*> { entryPoint });
    }
    lck.unlock();
    if (auto *logger = GetSimulator()->GetLogger())
        logger->Log(this,
                (::Xsmp::Helper::GetPath(entryPoint) + " subscribed to "
                        + event_name + ".").c_str(),
                ::Smp::Services::ILogger::LMK_Debug);
}

void XsmpEventManager::Unsubscribe(::Smp::Services::EventId event,
        const ::Smp::IEntryPoint *entryPoint) {

    auto event_name = GetEventName(event);

    std::scoped_lock lck { _subscriptionsMutex };

    if (auto it = _subscriptions.find(event); it != _subscriptions.end()) {

        auto &entryPoints = it->second;

        auto it2 = std::find(entryPoints.begin(), entryPoints.end(),
                entryPoint);
        if (it2 != entryPoints.end()) {
            entryPoints.erase(it2);
            if (auto *logger = GetSimulator()->GetLogger())
                logger->Log(this,
                        (::Xsmp::Helper::GetPath(entryPoint)
                                + " unsubscribed to " + event_name + ".").c_str(),
                        ::Smp::Services::ILogger::LMK_Debug);
            return;
        }
    }

    ::Xsmp::Exception::throwEntryPointNotSubscribed(this, entryPoint,
            event_name);
}

void XsmpEventManager::Emit(::Smp::Services::EventId event,
        ::Smp::Bool /*synchronous*/) {

    auto event_name = GetEventName(event);
    if (auto *logger = GetSimulator()->GetLogger())
        logger->Log(this, event_name.c_str(),
                ::Smp::Services::ILogger::LMK_Event);

    std::unique_lock lck { _subscriptionsMutex };
    auto it = _subscriptions.find(event);
    if (it != _subscriptions.end()) {
        // copy the entrypoints
        auto entryPoints = it->second;
        lck.unlock();
        // sync mode

        for (auto *entry_point : entryPoints)
            ::Xsmp::Helper::SafeExecute(GetSimulator(), entry_point);
    }
}

void XsmpEventManager::Restore(::Smp::IStorageReader *reader) {
    ::Xsmp::Persist::Restore(GetSimulator(), this, reader, _events,
            _subscriptions);
    //rebuild _ids map from _events
    _ids.clear();
    for (const auto& [name, id] : _events)
        _ids.try_emplace(id, name);
}

void XsmpEventManager::Store(::Smp::IStorageWriter *writer) {
    ::Xsmp::Persist::Store(GetSimulator(), this, writer, _events,
            _subscriptions);
}

} // namespace Xsmp::Services
