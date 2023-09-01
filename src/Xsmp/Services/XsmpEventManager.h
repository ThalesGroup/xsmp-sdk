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

#ifndef XSMP_SERVICES_XSMPEVENTMANAGER_H_
#define XSMP_SERVICES_XSMPEVENTMANAGER_H_

#include <Smp/IEntryPoint.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/EventId.h>
#include <Xsmp/Services/XsmpEventManagerGen.h>
#include <unordered_map>
#include <mutex>
#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------
// ----------------------------------------------------------------------------

namespace Xsmp::Services {

class XsmpEventManager final: public XsmpEventManagerGen {
public:
    // ------------------------------------------------------------------------------------
    // -------------------------- Constructors/Destructor --------------------------
    // ------------------------------------------------------------------------------------

    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    XsmpEventManager(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, ::Smp::ISimulator *simulator);

    /// Virtual destructor to release memory.
    ~XsmpEventManager() noexcept override = default;

    /// Get unique event identifier for an event name.
    /// It is guaranteed that this method will always return the same
    /// value when called with the same event name. This holds for
    /// pre-defined event names as well as for user-defined events.
    /// @param   eventName Name of the global event.
    /// @return  Event identifier for global event with given name.
    /// @throws  ::Smp::Services::InvalidEventName
    ::Smp::Services::EventId QueryEventId(::Smp::String8 eventName) override;

    /// Subscribe entry point to a global event.
    /// This method raises an exception of type InvalidEventId when
    /// called with an invalid event identifier. When the entry point
    /// is already subscribed to the same event, an exception of type
    /// EntryPointAlreadySubscribed is raised.
    ///
    /// An entry point can only be subscribed once to an event.
    /// @param   event Event identifier of global event to subscribe to.
    /// @param   entryPoint Entry point to subscribe to global event.
    /// @throws  ::Smp::Services::EntryPointAlreadySubscribed
    /// @throws  ::Smp::Services::InvalidEventId
    void Subscribe(::Smp::Services::EventId event,
            const ::Smp::IEntryPoint *entryPoint) override;

    /// Unsubscribe entry point from a global event.
    /// This method raises an exception of type InvalidEventId when
    /// called with an invalid event identifier. When the entry point
    /// is not subscribed to the event, an exception of type
    /// EntryPointNotSubscribed is raised.
    /// An entry point can only be unsubscribed from an event when it
    /// has been subscribed earlier using Subscribe().
    /// @param   event Event identifier of global event to unsubscribe
    ///          from.
    /// @param   entryPoint Entry point to unsubscribe from global event.
    /// @throws  ::Smp::Services::EntryPointNotSubscribed
    /// @throws  ::Smp::Services::InvalidEventId
    void Unsubscribe(::Smp::Services::EventId event,
            const ::Smp::IEntryPoint *entryPoint) override;

    /// Emit a global event.
    /// This will call all entry points that are subscribed to the
    /// global event with the given identifier at the time Emit() is
    /// called. Entry point subscription/unsubscription during the
    /// execution of Emit() is taken into account the next time Emit()
    /// is called. Entry points will be called in the order they have
    /// been subscribed to the global event.
    /// Only the simulation environment itself is allowed to emit
    /// events for changes of the simulator state. While an event for
    /// such a state transition is emitted, subscribed event handlers
    /// are not allowed to call another state transition of the
    /// simulator.
    /// @param   event Event identifier of global event to emit.
    /// @param   synchronous Flag whether to emit the given event
    ///          synchronously (the default) or asynchronously.
    /// @throws  ::Smp::Services::InvalidEventId
    void Emit(::Smp::Services::EventId event, ::Smp::Bool synchronous = true)
            override;

    void Restore(::Smp::IStorageReader *reader) override;

    void Store(::Smp::IStorageWriter *writer) override;
private:
    friend class ::Xsmp::Component::Helper;
    std::mutex _eventsMutex { };
    std::unordered_map<std::string, ::Smp::Services::EventId> _events { };

    mutable std::mutex _idsMutex { };
    std::unordered_map<::Smp::Services::EventId, const std::string&> _ids { };

    std::mutex _subscriptionsMutex { };
    std::unordered_map<::Smp::Services::EventId,
            std::vector<const ::Smp::IEntryPoint*>> _subscriptions { };

    const std::string& GetEventName(::Smp::Services::EventId event) const;

};
} // namespace Xsmp::Services

#endif // XSMP_SERVICES_XSMPEVENTMANAGER_H_
