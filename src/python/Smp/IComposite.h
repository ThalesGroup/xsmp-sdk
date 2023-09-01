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
#include <Smp/IComposite.h>
#include <string>
#include <typeinfo>

inline void RegisterIComposite(const py::module_ &m) {
    py::class_<::Smp::IComposite, ::Smp::IObject>(m, "IComposite",
            py::multiple_inheritance())

    .def("GetContainer", &::Smp::IComposite::GetContainer, py::arg("name"),
            py::return_value_policy::reference,
            R"(Query for a container of this composite by its name.
The returned container may be null if no container with the given name could be found.)")

    .doc() =
            R"(Interface for a composite.
An object with components as children implements this interface. Child components are held in named containers.)";
}

