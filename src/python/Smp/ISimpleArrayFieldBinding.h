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

#ifndef PYTHON_SMP_ISIMPLEARRAYFIELD_H_
#define PYTHON_SMP_ISIMPLEARRAYFIELD_H_

#include <python/ecss_smp.h>
#include <Smp/AnySimple.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IArrayType.h>
#include <string>

inline void RegisterISimpleArrayField(const py::module_ &m) {

    py::class_<::Smp::ISimpleArrayField, ::Smp::IField>(m, "ISimpleArrayField",
            py::multiple_inheritance())

    .def("__len__", &::Smp::ISimpleArrayField::GetSize)

    .def("__getitem__",
            [](const ::Smp::ISimpleArrayField &self, ::Smp::Int64 index) {
                try {
                    return convert(
                            self.GetValue(GetIndex(index, self.GetSize())));
                }
                catch (const std::exception&) {
                    throw py::index_error(std::to_string(index));
                }
            })

    .def("__setitem__",
            [](::Smp::ISimpleArrayField &self, ::Smp::UInt64 index,
                    const py::handle &value) {

                if (const auto *type =
                        dynamic_cast<const ::Smp::Publication::IArrayType*>(self.GetType()))
                    return self.SetValue(index,
                            convert(value,
                                    type->GetItemType()->GetPrimitiveTypeKind()));

                throw py::value_error("Could not find Item PrimitiveTypeKind.");
            })

    .doc() = "Interface to an array where each array item is of a simple type.";
}

#endif // PYTHON_SMP_ISIMPLEARRAYFIELD_H_
