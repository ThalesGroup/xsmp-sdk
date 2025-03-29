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

#include <Smp/IComposite.h>
#include <Smp/ISimulator.h>
#include <Smp/IStorageReader.h>
#include <Smp/IStorageWriter.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/EventId.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ILogger.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Persist.h>
#include <Xsmp/Persist/SmpIObject.h>
#include <Xsmp/Persist/StdString.h>
#include <Xsmp/Persist/StdUnorderedMap.h>
#include <Xsmp/Persist/StdVector.h>
#include <Xsmp/Services/XsmpEventManager.h>
#include <Xsmp/Services/XsmpEventManagerGen.h>
#include <algorithm>
#include <mutex>
#include <string>
#include <vector>

namespace Xsmp::Services {

using namespace ::Smp::Services;
XsmpEventManager::XsmpEventManager(::Smp::String8 name,
                                   ::Smp::String8 description,
                                   ::Smp::IComposite *parent,
                                   ::Smp::ISimulator *simulator)
    : XsmpEventManagerGen::XsmpEventManagerGen(name, description, parent,
                                               simulator) {

  auto eventsAccess = _events.write();
  eventsAccess.get().try_emplace(IEventManager::SMP_LeaveConnecting,
                                 IEventManager::SMP_LeaveConnectingId);
  eventsAccess.get().try_emplace(IEventManager::SMP_EnterInitialising,
                                 IEventManager::SMP_EnterInitialisingId);
  eventsAccess.get().try_emplace(IEventManager::SMP_LeaveInitialising,
                                 IEventManager::SMP_LeaveInitialisingId);
  eventsAccess.get().try_emplace(IEventManager::SMP_EnterStandby,
                                 IEventManager::SMP_EnterStandbyId);
  eventsAccess.get().try_emplace(IEventManager::SMP_LeaveStandby,
                                 IEventManager::SMP_LeaveStandbyId);
  eventsAccess.get().try_emplace(IEventManager::SMP_EnterExecuting,
                                 IEventManager::SMP_EnterExecutingId);
  eventsAccess.get().try_emplace(IEventManager::SMP_LeaveExecuting,
                                 IEventManager::SMP_LeaveExecutingId);
  eventsAccess.get().try_emplace(IEventManager::SMP_EnterStoring,
                                 IEventManager::SMP_EnterStoringId);
  eventsAccess.get().try_emplace(IEventManager::SMP_LeaveStoring,
                                 IEventManager::SMP_LeaveStoringId);
  eventsAccess.get().try_emplace(IEventManager::SMP_EnterRestoring,
                                 IEventManager::SMP_EnterRestoringId);
  eventsAccess.get().try_emplace(IEventManager::SMP_LeaveRestoring,
                                 IEventManager::SMP_LeaveRestoringId);
  eventsAccess.get().try_emplace(IEventManager::SMP_EnterExiting,
                                 IEventManager::SMP_EnterExitingId);
  eventsAccess.get().try_emplace(IEventManager::SMP_EnterAborting,
                                 IEventManager::SMP_EnterAbortingId);
  eventsAccess.get().try_emplace(IEventManager::SMP_EpochTimeChanged,
                                 IEventManager::SMP_EpochTimeChangedId);
  eventsAccess.get().try_emplace(IEventManager::SMP_MissionTimeChanged,
                                 IEventManager::SMP_MissionTimeChangedId);
  eventsAccess.get().try_emplace(IEventManager::SMP_EnterReconnecting,
                                 IEventManager::SMP_EnterReconnectingId);
  eventsAccess.get().try_emplace(IEventManager::SMP_LeaveReconnecting,
                                 IEventManager::SMP_LeaveReconnectingId);
  eventsAccess.get().try_emplace(IEventManager::SMP_PreSimTimeChange,
                                 IEventManager::SMP_PreSimTimeChangeId);
  eventsAccess.get().try_emplace(IEventManager::SMP_PostSimTimeChange,
                                 IEventManager::SMP_PostSimTimeChangeId);

  auto idsAccess = _ids.write();
  for (const auto &[eventName, id] : eventsAccess.get()) {
    idsAccess.get().try_emplace(id, eventName);
  }
}

::Smp::Services::EventId
XsmpEventManager::QueryEventId(::Smp::String8 eventName) {
  if (!eventName || eventName[0] == '\0') {
    ::Xsmp::Exception::throwInvalidEventName(this, eventName);
  }
  auto eventsAccess = _events.write();

  if (auto it = eventsAccess.get().find(eventName);
      it != eventsAccess.get().end()) {
    return it->second;
  }

  auto eventId =
      static_cast<::Smp::Services::EventId>(eventsAccess.get().size() + 1);

  const auto &name =
      eventsAccess.get().try_emplace(eventName, eventId).first->first;
  eventsAccess.unlock();
  _ids.write().get().try_emplace(eventId, name);
  return eventId;
}

const std::string &
XsmpEventManager::GetEventName(::Smp::Services::EventId event) const {
  auto idsAccess = _ids.read();
  auto it = idsAccess.get().find(event);
  if (it == idsAccess.get().end()) {
    ::Xsmp::Exception::throwInvalidEventId(this, event);
  }
  return it->second;
}

void XsmpEventManager::Subscribe(::Smp::Services::EventId event,
                                 const ::Smp::IEntryPoint *entryPoint) {

  const auto &event_name = GetEventName(event);
  {
    auto subscriptionAccess = _subscriptions.write();
    if (auto it = subscriptionAccess.get().find(event);
        it != subscriptionAccess.get().end()) {
      auto &entryPoints = it->second;

      if (std::find(entryPoints.begin(), entryPoints.end(), entryPoint) !=
          entryPoints.end()) {
        ::Xsmp::Exception::throwEntryPointAlreadySubscribed(this, entryPoint,
                                                            event_name);
      }
      entryPoints.push_back(entryPoint);
    } else {
      subscriptionAccess.get().try_emplace(
          event, std::vector<const ::Smp::IEntryPoint *>{entryPoint});
    }
  }
  if (auto *logger = GetSimulator()->GetLogger()) {
    logger->Log(this,
                (::Xsmp::Helper::GetPath(entryPoint) + " subscribed to " +
                 event_name + ".")
                    .c_str(),
                ::Smp::Services::ILogger::LMK_Debug);
  }
}

void XsmpEventManager::Unsubscribe(::Smp::Services::EventId event,
                                   const ::Smp::IEntryPoint *entryPoint) {

  const auto &event_name = GetEventName(event);

  auto subscriptionAccess = _subscriptions.write();
  if (auto it = subscriptionAccess.get().find(event);
      it != subscriptionAccess.get().end()) {

    auto &entryPoints = it->second;
    auto it2 = std::find(entryPoints.begin(), entryPoints.end(), entryPoint);
    if (it2 != entryPoints.end()) {
      entryPoints.erase(it2);
      if (auto *logger = GetSimulator()->GetLogger()) {
        logger->Log(this,
                    (::Xsmp::Helper::GetPath(entryPoint) + " unsubscribed to " +
                     event_name + ".")
                        .c_str(),
                    ::Smp::Services::ILogger::LMK_Debug);
      }
      return;
    }
  }

  ::Xsmp::Exception::throwEntryPointNotSubscribed(this, entryPoint, event_name);
}

void XsmpEventManager::Emit(::Smp::Services::EventId event,
                            ::Smp::Bool /*synchronous*/) {

  const auto &event_name = GetEventName(event);
  if (auto *logger = GetSimulator()->GetLogger()) {
    logger->Log(this, event_name.c_str(), ::Smp::Services::ILogger::LMK_Event);
  }
  auto subscriptionAccess = _subscriptions.read();
  if (auto it = subscriptionAccess.get().find(event);
      it != subscriptionAccess.get().end()) {
    // copy the entrypoints
    auto entryPoints = it->second;
    subscriptionAccess.unlock();
    for (const auto *entry_point : entryPoints) {
      ::Xsmp::Helper::SafeExecute(GetSimulator(), entry_point);
    }
  }
}

void XsmpEventManager::Restore(::Smp::IStorageReader *reader) {
  ::Xsmp::Persist::Restore(GetSimulator(), this, reader, _events.write().get(),
                           _subscriptions.write().get());
  // rebuild _ids map from _events
  auto idsAccess = _ids.write();
  idsAccess.get().clear();
  for (const auto &[name, id] : _events.read().get()) {
    idsAccess.get().try_emplace(id, name);
  }
}

void XsmpEventManager::Store(::Smp::IStorageWriter *writer) {
  ::Xsmp::Persist::Store(GetSimulator(), this, writer, _events.read().get(),
                         _subscriptions.read().get());
}

} // namespace Xsmp::Services
