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

#ifndef PYTHON_SMP_IARRAYFIELD_H_
#define PYTHON_SMP_IARRAYFIELD_H_

#include <Smp/IArrayField.h>
#include <Smp/PrimitiveTypes.h>
#include <python/ecss_smp.h>
#include <string>

inline void RegisterIArrayField(const py::module_ &m) {
  py::class_<::Smp::IArrayField, ::Smp::IField>(m, "IArrayField",
                                                py::multiple_inheritance())

      .def("__len__", &::Smp::IArrayField::GetSize)

      .def(
          "__getitem__",
          [](const ::Smp::IArrayField &self, ::Smp::Int64 index) {
            if (auto *result = self.GetItem(GetIndex(index, self.GetSize())))
              return result;
            throw py::index_error(std::to_string(index));
          },
          py::return_value_policy::reference)

      .def("GetSize", &::Smp::IArrayField::GetSize,
           "Get the size (number of array items) of the field.")

      .def("GetItem", &::Smp::IArrayField::GetItem,
           py::return_value_policy::reference, py::arg("index"),
           "Get an array item by index.")

      .doc() = "Interface of a field which is of array type.";
}

#endif // PYTHON_SMP_IARRAYFIELD_H_
