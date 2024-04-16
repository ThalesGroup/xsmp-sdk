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

#ifndef PYTHON_SMP_PUBLICATION_ITYPEREGISTRY_H_
#define PYTHON_SMP_PUBLICATION_ITYPEREGISTRY_H_

#include <Smp/Publication/ITypeRegistry.h>
#include <python/ecss_smp.h>

inline void RegisterITypeRegistry(const py::module_ &m) {
  py::class_<::Smp::Publication::ITypeRegistry>(m, "ITypeRegistry",
                                                py::multiple_inheritance())

      .def("GetType",
           py::overload_cast<::Smp::PrimitiveTypeKind>(
               &::Smp::Publication::ITypeRegistry::GetType, py::const_),
           py::arg("type"), py::return_value_policy::reference,
           "Returns a type by its primitive type kind.")

      .def("GetType",
           py::overload_cast<::Smp::Uuid>(
               &::Smp::Publication::ITypeRegistry::GetType, py::const_),
           py::arg("typeUuid"), py::return_value_policy::reference,
           "Returns a type by universally unique identifier.")

      .def("AddFloatType", &::Smp::Publication::ITypeRegistry::AddFloatType,
           py::arg("name"), py::arg("description"), py::arg("typeUuid"),
           py::arg("minimum"), py::arg("maximum"), py::arg("minInclusive"),
           py::arg("maxInclusive"), py::arg("unit"),
           py::arg("type") = ::Smp::PrimitiveTypeKind::PTK_Float64,
           py::return_value_policy::reference,
           "Add a float type to the registry. IComponent and "
           "IDynamicInvocation support fields, parameters and operations of "
           "Float types via the PTK_Float32 and PTK_Float64 primitive type, as "
           "a Float is mapped either to Float32 or Float64.")

      .def("AddIntegerType", &::Smp::Publication::ITypeRegistry::AddIntegerType,
           py::arg("name"), py::arg("description"), py::arg("typeUuid"),
           py::arg("minimum"), py::arg("maximum"), py::arg("unit"),
           py::arg("type") = ::Smp::PrimitiveTypeKind::PTK_Int32,
           py::return_value_policy::reference,
           "Add an integer type to the registry. IComponent and "
           "IDynamicInvocation support fields, parameters and operations of "
           "Integer types via the PTK_Int primitive types, as an Integer is "
           "mapped to one of Int8 / Int16 / Int32 / Int64 / UInt8 / UInt16 / "
           "UInt32 / UInt64.")

      .def("AddEnumerationType",
           &::Smp::Publication::ITypeRegistry::AddEnumerationType,
           py::arg("name"), py::arg("description"), py::arg("typeUuid"),
           py::arg("memorySize") = 4, py::return_value_policy::reference,
           "Add an enumeration type to the registry.")

      .def("AddArrayType", &::Smp::Publication::ITypeRegistry::AddArrayType,
           py::arg("name"), py::arg("description"), py::arg("typeUuid"),
           py::arg("itemTypeUuid"), py::arg("itemSize"), py::arg("arrayCount"),
           py::arg("simpleArray") = false, py::return_value_policy::reference,
           "Add an array type to the registry.")

      .def("AddStringType", &::Smp::Publication::ITypeRegistry::AddStringType,
           py::arg("name"), py::arg("description"), py::arg("typeUuid"),
           py::arg("length"), py::return_value_policy::reference,
           "Add a string type to the registry.")

      .def("AddStructureType",
           &::Smp::Publication::ITypeRegistry::AddStructureType,
           py::arg("name"), py::arg("description"), py::arg("typeUuid"),
           py::return_value_policy::reference,
           "Add a structure type to the registry.")

      .def("AddClassType", &::Smp::Publication::ITypeRegistry::AddClassType,
           py::arg("name"), py::arg("description"), py::arg("typeUuid"),
           py::arg("baseClassUuid"), py::return_value_policy::reference,
           "Add a class type to the registry.")

      .doc() =
      R"(This interface defines a registration mechanism for user defined types.
Via the IPublication interface (that has to be provided to each component during the Publish state transition), it is mandatory that a simulator provides a type registry.
The simulator has to ensure that this type registry contains at least all pre-defined value types of the SMP standard, using their pre-defined universally unique identifiers.
However, it is not mandatory for models to make use of this type registry.)";
}

#endif // PYTHON_SMP_PUBLICATION_ITYPEREGISTRY_H_
