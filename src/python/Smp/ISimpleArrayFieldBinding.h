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

#include <Smp/AnySimple.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IArrayType.h>
#include <python/ecss_smp.h>
#include <string>
#include <vector>

inline void RegisterISimpleArrayField(const py::module_ &m) {

  py::class_<::Smp::ISimpleArrayField, ::Smp::IField>(
      m, "ISimpleArrayField", py::multiple_inheritance())

      .def("__len__", &::Smp::ISimpleArrayField::GetSize)

      .def("__getitem__",
           [](const ::Smp::ISimpleArrayField &self, ::Smp::Int64 index) {
             try {
               return convert(self.GetValue(GetIndex(index, self.GetSize())));
             } catch (const std::exception &) {
               throw py::index_error(std::to_string(index));
             }
           })

      // a slice maps onto GetValues(), which reads a run of items in one
      // call from startIndex
      .def(
          "__getitem__",
          [](const ::Smp::ISimpleArrayField &self, const py::slice &slice) {
            size_t start = 0;
            size_t stop = 0;
            size_t step = 0;
            size_t length = 0;
            if (!slice.compute(self.GetSize(), &start, &stop, &step, &length)) {
              throw py::error_already_set();
            }
            py::list result{length};
            if (length == 0) {
              return result;
            }
            // GetValues() reads contiguously, so only a unit step can use it
            if (step == 1) {
              std::vector<::Smp::AnySimple> values{length};
              self.GetValues(length, values.data(), start);
              for (size_t i = 0; i < length; ++i) {
                result[i] = convert(values[i]);
              }
            } else {
              for (size_t i = 0; i < length; ++i, start += step) {
                result[i] = convert(self.GetValue(start));
              }
            }
            return result;
          })

      .def("__setitem__",
           [](::Smp::ISimpleArrayField &self, ::Smp::UInt64 index,
              const py::handle &value) {
             if (const auto *type =
                     dynamic_cast<const ::Smp::Publication::IArrayType *>(
                         self.GetType()))
               return self.SetValue(
                   index,
                   convert(value, type->GetItemType()->GetPrimitiveTypeKind()));

             throw py::value_error("Could not find Item PrimitiveTypeKind.");
           })

      .def(
          "__setitem__",
          [](::Smp::ISimpleArrayField &self, const py::slice &slice,
             const py::sequence &values) {
            const auto *type =
                dynamic_cast<const ::Smp::Publication::IArrayType *>(
                    self.GetType());
            if (!type) {
              throw py::value_error("Could not find Item PrimitiveTypeKind.");
            }
            const auto kind = type->GetItemType()->GetPrimitiveTypeKind();
            size_t start = 0;
            size_t stop = 0;
            size_t step = 0;
            size_t length = 0;
            if (!slice.compute(self.GetSize(), &start, &stop, &step, &length)) {
              throw py::error_already_set();
            }
            if (static_cast<size_t>(py::len(values)) != length) {
              throw py::value_error(
                  "The number of values does not match the slice.");
            }
            if (step == 1) {
              std::vector<::Smp::AnySimple> converted;
              converted.reserve(length);
              for (const auto &value : values) {
                converted.emplace_back(convert(value, kind));
              }
              self.SetValues(length, converted.data(), start);
            } else {
              for (size_t i = 0; i < length; ++i, start += step) {
                self.SetValue(start, convert(values[i], kind));
              }
            }
          })

      .doc() =
      "Interface to an array where each array item is of a simple type.";
}

#endif // PYTHON_SMP_ISIMPLEARRAYFIELD_H_
