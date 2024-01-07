// Copyright 2024 THALES ALENIA SPACE FRANCE. All rights reserved.
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

#ifndef PYTHON_SMP_IFACTORY_H_
#define PYTHON_SMP_IFACTORY_H_

#include <python/ecss_smp.h>
#include <Smp/IFactory.h>

inline void RegisterIFactory(const py::module_ &m) {
    py::class_<::Smp::IFactory, ::Smp::IObject>(m, "IFactory",
            py::multiple_inheritance())

    .def("GetUuid", &::Smp::IFactory::GetUuid,
            "Get Universally unique identifier of the type instantiated by the factory.")

    .def("GetTypeName", &::Smp::IFactory::GetTypeName,
            "Returns the fully qualified C++ name of the type.")

    .def("CreateInstance", &::Smp::IFactory::CreateInstance, py::arg("name"),
            py::arg("description"), py::arg("parent"),
            py::return_value_policy::reference,
            "Create a new instance with given name, description and parent.")

    .def("DeleteInstance", &::Smp::IFactory::DeleteInstance,
            py::arg("instance"), "Delete an existing instance.")

    .doc() = "Interface for a component factory.";
}

#endif // PYTHON_SMP_IFACTORY_H_
