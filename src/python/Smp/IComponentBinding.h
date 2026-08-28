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

#ifndef PYTHON_SMP_ICOMPONENT_H_
#define PYTHON_SMP_ICOMPONENT_H_

#include <Smp/AnySimple.h>
#include <Smp/ICollectionBase.h>
#include <Smp/IComponent.h>
#include <Smp/IField.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/Publication/IType.h>
#include <python/ecss_smp.h>
#include <vector>

inline void RegisterIComponent(const py::module_ &m) {
  py::class_<::Smp::IComponent, ::Smp::IObject>(m, "IComponent",
                                                py::multiple_inheritance())

      .def("GetState", &::Smp::IComponent::GetState,
           R"(Returns the state the component is currently in.
The component state can be changed using the Publish(), Configure()and Connect() state transition methods.)")

      .def("GetFields", &::Smp::IComponent::GetFields,
           py::return_value_policy::reference_internal,
           "Get the collection of all fields published by the component.")

      .def("GetField", &::Smp::IComponent::GetField,
           py::return_value_policy::reference_internal, py::arg("full_name"),
           R"(Get the field of given name.
This method raises an exception of type InvalidFieldName if called with a field name for which no corresponding field exists.
This method can be used both for fields of simple types (when it returns an instance of ISimpleField), and for complex fields (when it returns IArrayField or IStructureField).)")

      .def("GetUuid", &::Smp::IComponent::GetUuid,
           "Get Universally Unique Identifier of Component Type.")

      .def("GetFields", &::Smp::IComponent::GetFields,
           "Returns the collection of the fields of the component.",
           py::return_value_policy::reference_internal)

      .def(
          "GetSimpleValue",
          [](const ::Smp::IComponent &self, ::Smp::String8 fullName) {
            return convert(self.GetSimpleValue(fullName));
          },
          py::arg("full_name"),
          R"(Get the value of a field of simple type.
This method raises an exception of type InvalidFieldName when there is no such field, or when it is not of a simple type.)")

      .def(
          "SetSimpleValue",
          [](::Smp::IComponent &self, ::Smp::String8 fullName,
             const py::handle &value) {
            auto *field =
                dynamic_cast<::Smp::ISimpleField *>(self.GetField(fullName));
            if (!field) {
              throw py::key_error(fullName);
            }
            self.SetSimpleValue(fullName,
                                convert(value, field->GetPrimitiveTypeKind()));
          },
          py::arg("full_name"), py::arg("value"),
          "Set the value of a field of simple type.")

      .def(
          "GetSimpleArrayValue",
          [](const ::Smp::IComponent &self, ::Smp::String8 fullName,
             ::Smp::UInt64 length, ::Smp::UInt64 startIndex) {
            std::vector<::Smp::AnySimple> values(length);
            self.GetSimpleArrayValue(fullName, length, values.data(),
                                     startIndex);
            py::list result;
            for (const auto &value : values) {
              result.append(convert(value));
            }
            return result;
          },
          py::arg("full_name"), py::arg("length"), py::arg("start_index") = 0,
          "Get the values of a simple array field, as a list of length items "
          "read from start_index.")

      .def(
          "SetSimpleArrayValue",
          [](::Smp::IComponent &self, ::Smp::String8 fullName,
             const py::sequence &values, ::Smp::UInt64 startIndex) {
            auto *field = dynamic_cast<::Smp::ISimpleArrayField *>(
                self.GetField(fullName));
            if (!field) {
              throw py::key_error(fullName);
            }
            const auto kind = field->GetType()->GetPrimitiveTypeKind();
            std::vector<::Smp::AnySimple> converted;
            converted.reserve(values.size());
            for (const auto &value : values) {
              converted.emplace_back(convert(value, kind));
            }
            self.SetSimpleArrayValue(fullName, converted.size(),
                                     converted.data(), startIndex);
          },
          py::arg("full_name"), py::arg("values"), py::arg("start_index") = 0,
          "Set the values of a simple array field from a sequence, starting at "
          "start_index.")

      .def("AddChild", &::Smp::IComponent::AddChild, py::arg("child"),
           py::arg("collection"),
           "Register a new child object under the component. Returns False "
           "when its name is already taken.")

      .def("RemoveChild", &::Smp::IComponent::RemoveChild, py::arg("child"),
           py::arg("collection"),
           "Unregister a child object from the component. Returns False when "
           "it was not registered with that collection.")

      .def("IsChildInCollection", &::Smp::IComponent::IsChildInCollection,
           py::arg("child"), py::arg("collection"),
           py::return_value_policy::reference_internal,
           "Look a name up among the children registered for a collection.")

      .doc() =
      R"(This is the base interface for all SMP components.
All components need to implement this interface. As components interface to the simulation environment, they have a dependency to it via the two interfaces IPublication and ISimulator.
This is the only mandatory interface components have to implement. All other interfaces are optional.)";
}

#endif // PYTHON_SMP_ICOMPONENT_H_
