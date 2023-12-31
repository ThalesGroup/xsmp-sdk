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

#ifndef PYTHON_SMP_IEVENTSINK_H_
#define PYTHON_SMP_IEVENTSINK_H_

#include <python/ecss_smp.h>
#include <Smp/IEventSink.h>
#include <Smp/IEventSource.h>

::Smp::IEventSink& SubscribeEventSink(::Smp::IEventSink &self,
        ::Smp::IEventSource &eso) {
    eso.Subscribe(&self);
    return self;
}
::Smp::IEventSink& UnsubscribeEventSink(::Smp::IEventSink &self,
        ::Smp::IEventSource &eso) {
    eso.Unsubscribe(&self);
    return self;
}

inline void RegisterIEventSink(const py::module_ &m) {
    py::class_<::Smp::IEventSink, ::Smp::IObject>(m, "IEventSink",
            py::multiple_inheritance())

    .def("Subscribe", &SubscribeEventSink, py::arg("event_source"),
            R"(Subscribe to the event source, i.e. request notifications.
If the given event sink is already subscribed to the event source,  an exception of type EventSinkAlreadySubscribed is thrown.
If the type of the event argument of the event sink is not the type the event source expects, an exception of type InvalidEventSink is thrown.
An event sink can only be subscribed once to each event source. Event sinks will be called in the order they have been subscribed to the event source.)")

    .def("__iadd__", &SubscribeEventSink, py::arg("event_source"),
            py::return_value_policy::reference)

    .def("Unsubscribe", &UnsubscribeEventSink, py::arg("event_source"),
            R"(Unsubscribe from the event source, i.e. cancel notifications.
This method raises the EventSinkNotSubscribed exception if the given event sink is not subscribed to the event source.
An event sink can only be unsubscribed if it has been subscribed before.)")

    .def("__isub__", &UnsubscribeEventSink, py::arg("event_source"),
            py::return_value_policy::reference)

    .doc() =
            R"(Interface of an event sink that can be subscribed to an event source (IEventSource).
This interface provides a notification method (event handler) that can be called by event sources when an event is emitted.)";
}

#endif // PYTHON_SMP_IEVENTSINK_H_
