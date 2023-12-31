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

#ifndef PYTHON_SMP_IFAILURE_H_
#define PYTHON_SMP_IFAILURE_H_

#include <python/ecss_smp.h>
#include <Smp/IFailure.h>

inline void RegisterIFailure(const py::module_ &m) {
    py::class_<::Smp::IFailure, ::Smp::IPersist>(m, "IFailure",
            py::multiple_inheritance())

    .def("Fail", &::Smp::IFailure::Fail,
            "Sets the state of the failure to failed.")

    .def("Unfail", &::Smp::IFailure::Unfail,
            "Sets the state of the failure to unfailed.")

    .def("IsFailed", &::Smp::IFailure::IsFailed,
            "Returns whether the failure's state is set to failed.")

    .doc() =
            R"(Interface for a failure.
A Failure allows to query and to set its state to Failed or Unfailed. 
Failures can be exposed via the IFallibleModel interface.)";
}

#endif // PYTHON_SMP_IFAILURE_H_
