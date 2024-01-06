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

#ifndef PYTHON_SMP_PUBLICATION_IENUMERATIONTYPE_H_
#define PYTHON_SMP_PUBLICATION_IENUMERATIONTYPE_H_

#include <python/ecss_smp.h>
#include <Smp/Publication/IEnumerationType.h>

inline void RegisterIEnumerationType(const py::module_ &m) {
    py::class_<::Smp::Publication::IEnumerationType, ::Smp::Publication::IType>(m,
            "IEnumerationType", py::multiple_inheritance())

    .def("AddLiteral", &::Smp::Publication::IEnumerationType::AddLiteral,
            R"(Get the size (number of array items) of the array type.)")



    .doc() = R"(This interface defines a user defined enumeration type.)";
}

#endif // PYTHON_SMP_PUBLICATION_IENUMERATIONTYPE_H_
