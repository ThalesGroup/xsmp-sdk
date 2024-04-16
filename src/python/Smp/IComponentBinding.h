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

#include <Smp/IComponent.h>
#include <python/ecss_smp.h>

inline void RegisterIComponent(const py::module_ &m) {
  py::class_<::Smp::IComponent, ::Smp::IObject>(m, "IComponent",
                                                py::multiple_inheritance())

      .def("GetState", &::Smp::IComponent::GetState,
           R"(Returns the state the component is currently in.
The component state can be changed using the Publish(), Configure()and Connect() state transition methods.)")

      .def("GetField", &::Smp::IComponent::GetField, py::arg("full_name"),
           R"(Get the field of given name.
This method raises an exception of type InvalidFieldName if called with a field name for which no corresponding field exists.
This method can be used both for fields of simple types (when it returns an instance of ISimpleField), and for complex fields (when it returns IArrayField or IStructureField).)")

      .def("GetUuid", &::Smp::IComponent::GetUuid,
           "Get Universally Unique Identifier of Component Type.")

      .doc() =
      R"(This is the base interface for all SMP components.
All components need to implement this interface. As components interface to the simulation environment, they have a dependency to it via the two interfaces IPublication and ISimulator.
This is the only mandatory interface components have to implement. All other interfaces are optional.)";
}

#endif // PYTHON_SMP_ICOMPONENT_H_
