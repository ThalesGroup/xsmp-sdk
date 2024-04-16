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

#ifndef PYTHON_SMP_ILINKINGCOMPONENT_H_
#define PYTHON_SMP_ILINKINGCOMPONENT_H_

#include <Smp/ILinkingComponent.h>
#include <python/ecss_smp.h>

inline void RegisterILinkingComponent(const py::module_ &m) {
  py::class_<::Smp::ILinkingComponent, ::Smp::IComponent>(
      m, "ILinkingComponent", py::multiple_inheritance())

      .def(
          "RemoveLinks", &::Smp::ILinkingComponent::RemoveLinks,
          py::arg("target"),
          R"(Asks a component to remove all its links to the given target component.
After this method has been called, the component must not try to access the given target component anymore.)")

      .doc() =
      R"(Interface for a component which can hold links to other components.
This interface extends the component interface and adds a method which asks a component to remove all its links to another component. A link to another component is any form of dependency that requires the existence of another component.
Links include (but are not limited to) the following:
     - Interface links: An interface link is a link to an interface provided by another component.
     - Event links: An event link is a link to an event sink of another component.
     - Entry point links: An entry point link is a link to an entry point of another component.)";
}

#endif // PYTHON_SMP_ILINKINGCOMPONENT_H_
