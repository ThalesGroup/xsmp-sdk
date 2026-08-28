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

#ifndef PYTHON_SMP_PUBLICATION_ISTRINGTYPE_H_
#define PYTHON_SMP_PUBLICATION_ISTRINGTYPE_H_

#include <Smp/Publication/IStringType.h>
#include <python/ecss_smp.h>

inline void RegisterIStringType(const py::module_ &m) {
  py::class_<::Smp::Publication::IStringType, ::Smp::Publication::IType>(
      m, "IStringType", py::multiple_inheritance())

      .def("GetMaxLength", &::Smp::Publication::IStringType::GetMaxLength,
           "Get the maximum length of the string, excluding the last null "
           "character. A String type has memory space to contain at most "
           "(GetMaxLength + 1) bytes.")

      .doc() =
      R"(This interface defines a user defined string type.
A String type represents a null-terminated string that can contain a maximum number of Smp::Char8 characters. It is a type for which the IType GetPrimitiveTypeKind method returns PTK_String8.)";
}

#endif // PYTHON_SMP_PUBLICATION_ISTRINGTYPE_H_
