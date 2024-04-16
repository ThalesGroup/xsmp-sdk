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

#ifndef PYTHON_SMP_IEVENTSOURCE_H_
#define PYTHON_SMP_IEVENTSOURCE_H_

#include <Smp/IEventSource.h>
#include <python/ecss_smp.h>

::Smp::IEventSource &SubscribeEventSource(::Smp::IEventSource &self,
                                          ::Smp::IEventSink &esi) {
  self.Subscribe(&esi);
  return self;
}
::Smp::IEventSource &UnsubscribeEventSource(::Smp::IEventSource &self,
                                            ::Smp::IEventSink &esi) {
  self.Unsubscribe(&esi);
  return self;
}

inline void RegisterIEventSource(const py::module_ &m) {
  py::class_<::Smp::IEventSource, ::Smp::IObject>(m, "IEventSource",
                                                  py::multiple_inheritance())

      .def("Subscribe", &::Smp::IEventSource::Subscribe, py::arg("event_sink"),
           R"(Subscribe to the event source, i.e. request notifications.
If the given event sink is already subscribed to the event source,  an exception of type EventSinkAlreadySubscribed is thrown.
If the type of the event argument of the event sink is not the type the event source expects, an exception of type InvalidEventSink is thrown.
An event sink can only be subscribed once to each event source. Event sinks will be called in the order they have been subscribed to the event source.)")

      .def("__iadd__", &SubscribeEventSource, py::arg("event_sink"),
           py::return_value_policy::reference)

      .def("Unsubscribe", &::Smp::IEventSource::Unsubscribe,
           py::arg("event_sink"),
           R"(Unsubscribe from the event source, i.e. cancel notifications.
This method raises the EventSinkNotSubscribed exception if the given event sink is not subscribed to the event source.
An event sink can only be unsubscribed if it has been subscribed before.)")

      .def("__isub__", &UnsubscribeEventSource, py::arg("event_sink"),
           py::return_value_policy::reference)

      .doc() =
      R"(Interface of an event source that event sinks (IEventSink) can subscribe to.
This interface allows event consumers to subscribe to or unsubscribe from an event.)";
}

#endif // PYTHON_SMP_IEVENTSOURCE_H_
