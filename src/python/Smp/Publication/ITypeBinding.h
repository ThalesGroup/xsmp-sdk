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

#ifndef PYTHON_SMP_PUBLICATION_ITYPE_H_
#define PYTHON_SMP_PUBLICATION_ITYPE_H_

#include <python/ecss_smp.h>
#include <Smp/Publication/IType.h>

inline void RegisterIType(const py::module_ &m) {
    py::class_<::Smp::Publication::IType, ::Smp::IObject>(m, "IType",
            py::multiple_inheritance())

    .def("GetPrimitiveTypeKind",
            &::Smp::Publication::IType::GetPrimitiveTypeKind,
            R"(Get primitive type kind that this type maps to, or PTK_None when the type cannot be mapped to a primitive type.)")

    .def("GetUuid", &::Smp::Publication::IType::GetUuid,
            R"(Get Universally Unique Identifier of type.)")

    .doc() = R"(This base interface defines a type in the type registry.)";
}

#endif // PYTHON_SMP_PUBLICATION_ITYPE_H_
