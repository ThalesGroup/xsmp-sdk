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

#include <python/ecss_smp.h>
#include <Smp/IEventConsumer.h>
#include <string>
#include <typeinfo>

inline void RegisterIEventConsumer(const py::module_ &m) {
    py::class_<::Smp::IEventConsumer, ::Smp::IComponent>(m, "IEventConsumer",
            py::multiple_inheritance())

    .def("GetEventSink", &::Smp::IEventConsumer::GetEventSink, py::arg("name"),
            py::return_value_policy::reference,
            R"(Query for an event sink of this component by its name.
The returned event sink may be null if no event sink with the given name could be found.)")

    .doc() =
            R"(Interface of an event consumer.
An event consumer is a component that holds event sinks, which may be subscribed to other component's event sources.
This is an optional interface. It needs to be implemented by components which want to allow access to event sinks by name.)";
}

