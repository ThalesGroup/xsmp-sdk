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

#ifndef PYTHON_SMP_IEVENTPROVIDER_H_
#define PYTHON_SMP_IEVENTPROVIDER_H_

#include <python/ecss_smp.h>
#include <Smp/IEventProvider.h>
#include <string>
#include <typeinfo>

inline void RegisterIEventProvider(const py::module_ &m) {
    py::class_<::Smp::IEventProvider, ::Smp::IComponent>(m, "IEventProvider",
            py::multiple_inheritance())

    .def("GetEventSource", &::Smp::IEventProvider::GetEventSource,
            py::arg("name"), py::return_value_policy::reference,
            R"(Query for an event source of this component by its name.
The returned event source may be null if no event source with the given name could be found.)")

    .doc() =
            R"(Interface of an event provider.
An event provider is a component that holds event sources, which allow other components to subscribe their event sinks.
This is an optional interface. It needs to be implemented by components which want to allow access to event sources by name.)";
}

#endif // PYTHON_SMP_IEVENTPROVIDER_H_