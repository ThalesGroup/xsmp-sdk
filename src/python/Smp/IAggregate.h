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
#include <Smp/IAggregate.h>

inline void RegisterIAggregate(const py::module_ &m) {
    py::class_<::Smp::IAggregate, ::Smp::IComponent>(m, "IAggregate",
            py::multiple_inheritance())

    .def("GetReference", &::Smp::IAggregate::GetReference, py::arg("name"),
            py::return_value_policy::reference,
            R"(Query for a reference of this aggregate component by its name.
The returned reference may be null if no reference with the given name could be found. If more than one reference with this name exists, it is not defined which one is returned.)")

    .doc() =
            R"(Interface for an aggregate component.
A component with references to other components implements this interface. Referenced components are held in named references.)";
}

