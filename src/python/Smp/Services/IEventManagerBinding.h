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

#ifndef PYTHON_SMP_SERVICES_IEVENTMANAGER_H_
#define PYTHON_SMP_SERVICES_IEVENTMANAGER_H_

#include <python/ecss_smp.h>

#include <Smp/Services/IEventManager.h>

inline void RegisterIEventManager(const py::module_ &m) {

    py::class_<::Smp::Services::IEventManager, ::Smp::IService>(m,
            "IEventManager", py::multiple_inheritance())

    .def("QueryEventId", &::Smp::Services::IEventManager::QueryEventId,
            py::arg("event_name"),
            R"(Get unique event identifier for an event name.
It is guaranteed that this method will always return the same value when called with the same event name. This holds for pre-defined event names as well as for user-defined events.)")

    .def("Subscribe", &::Smp::Services::IEventManager::Subscribe,
            py::arg("event"), py::arg("entry_point"),
            R"(Subscribe entry point to a global event.
This method raises an exception of type InvalidEventId when called with an invalid event identifier.
When the entry point is already subscribed to the same event, an exception of type EntryPointAlreadySubscribed is raised.
An entry point can only be subscribed once to an event.)")

    .def("Unsubscribe", &::Smp::Services::IEventManager::Unsubscribe,
            py::arg("event"), py::arg("entry_point"),
            R"(Unsubscribe entry point from a global event.
This method raises an exception of type InvalidEventId when called with an invalid event identifier.
When the entry point is not subscribed to the event, an exception of type EntryPointNotSubscribed is raised.
An entry point can only be unsubscribed from an event when it has been subscribed earlier using Subscribe().)")

    .def("Emit", &::Smp::Services::IEventManager::Emit, py::arg("event"),
            py::arg("synchronous") = true,
            R"(Emit a global event.
This will call all entry points that are subscribed to the global event with the given identifier at the time Emit() is called.
Entry point subscription/unsubscription during the  execution of Emit() is taken into account the next time Emit() is called.
Entry points will be called in the order they have been subscribed to the global event.
Only the simulation environment itself is allowed to emit events for changes of the simulator state.
While an event for such a state transition is emitted, subscribed event handlers are not allowed to call another state transition of the simulator.)")

    .doc() =
            R"(This interface is implemented by the Event Manager Service.
Components can register entry points with events, and they can define and emit events.)";
}

#endif // PYTHON_SMP_SERVICES_IEVENTMANAGER_H_