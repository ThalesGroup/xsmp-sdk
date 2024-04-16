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

#ifndef PYTHON_SMP_ICOLLECTION_H_
#define PYTHON_SMP_ICOLLECTION_H_

#include <Smp/ICollection.h>
#include <Smp/PrimitiveTypes.h>
#include <cstddef>
#include <python/ecss_smp.h>
#include <string>

template <typename T>
inline void RegisterICollection(const py::module_ &m, ::Smp::String8 name) {
  py::class_<::Smp::ICollection<T>, ::Smp::IObject>(m, name,
                                                    py::multiple_inheritance())

      .def("__len__", &::Smp::ICollection<T>::size)

      .def("size", &::Smp::ICollection<T>::size,
           "Get the number of objects in the sequence.")

      .def("at",
           py::overload_cast<std::size_t>(&::Smp::ICollection<T>::at,
                                          py::const_),
           py::arg("index"), py::return_value_policy::reference,
           "Retrieve element by position in the sequence (based on order of "
           "insertion).")

      .def("at",
           py::overload_cast<::Smp::String8>(&::Smp::ICollection<T>::at,
                                             py::const_),
           py::arg("name"), py::return_value_policy::reference,
           "Retrieve element by name")

      .def(
          "__getitem__",
          [](const ::Smp::ICollection<T> &self, ::Smp::Int64 index) {
            if (auto *cmp = self.at(GetIndex(index, self.size())))
              return cmp;
            throw py::index_error(std::to_string(index));
          },
          py::return_value_policy::reference)

      .def(
          "__getitem__",
          [](const ::Smp::ICollection<T> &self, ::Smp::String8 name) {
            if (auto *cmp = self.at(name))
              return cmp;
            throw py::key_error(name);
          },
          py::return_value_policy::reference)

      .doc() =
      R"(Interface for a collection.
A collection allows querying for the contained elements.
A collection must enforce uniqueness of the names of the contained elements.
Elements in the collection can be queried by name and by position.
The query by position must always return the elements based on order of insertion.)";
}

#endif // PYTHON_SMP_ICOLLECTION_H_
