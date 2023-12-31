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

#ifndef PYTHON_SMP_IFALLIBLEMODEL_H_
#define PYTHON_SMP_IFALLIBLEMODEL_H_

#include <python/ecss_smp.h>
#include <Smp/IFallibleModel.h>

inline void RegisterIFallibleModel(const py::module_ &m) {
    py::class_<::Smp::IFallibleModel, ::Smp::IModel>(m, "IFallibleModel",
            py::multiple_inheritance())

    .def("IsFailed", &::Smp::IFallibleModel::IsFailed,
            "Query for whether the model is failed. A model is failed when at least one of its failures is failed.")

    .def("GetFailure", &::Smp::IFallibleModel::GetFailure, py::arg("name"),
            py::return_value_policy::reference,
            R"(Get a failure by name.
The returned failure may be null if no child with the given name could be found.)")

    .doc() =
            R"(Interface for a fallible model that exposes its failure state and a collection of failures.
A fallible model allows querying for its failures by name.)";
}

#endif // PYTHON_SMP_IFALLIBLEMODEL_H_
