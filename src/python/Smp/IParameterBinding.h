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

#ifndef PYTHON_SMP_IPARAMETER_H_
#define PYTHON_SMP_IPARAMETER_H_

#include <python/ecss_smp.h>
#include <Smp/IParameter.h>

inline void RegisterIParameter(const py::module_ &m) {
    py::class_<::Smp::IParameter, ::Smp::IObject>(m, "IParameter",
            py::multiple_inheritance())

    .def("GetType", &::Smp::IParameter::GetType,
            py::return_value_policy::reference,
            "Provides the type of the parameter.")

    .def("GetDirection", &::Smp::IParameter::GetDirection,
            "Provides the parameter direction kind of the parameter.")

    .doc() =
            R"(This interface describes a parameter of a published operation.)";
}

#endif // PYTHON_SMP_IPARAMETER_H_
