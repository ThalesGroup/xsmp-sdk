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

#ifndef PYTHON_SMP_VIEWKIND_H_
#define PYTHON_SMP_VIEWKIND_H_

#include <Smp/ViewKind.h>
#include <python/ecss_smp.h>

inline void RegisterViewKind(const py::module_ &m) {
  py::enum_<::Smp::ViewKind>(m, "ViewKind")

      .value(
          "VK_None", ::Smp::ViewKind::VK_None,
          "The element is not made visible to the user (this is the default).")

      .value("VK_Debug", ::Smp::ViewKind::VK_Debug,
             R"(The element is made visible for debugging purposes.
The element is not visible to end users. If the simulation infrastructure supports the selection of different user roles, then the element shall be visible to "Debug" users only.)")

      .value("VK_Expert", ::Smp::ViewKind::VK_Expert,
             R"(The element is made visible for expert users.
The element is not visible to end users. If the simulation infrastructure supports the selection of different user roles, then the element shall be visible to "Debug" and "Expert" users.)")

      .value("VK_All", ::Smp::ViewKind::VK_All,
             "The element is made visible to all users.");
}

#endif // PYTHON_SMP_VIEWKIND_H_
