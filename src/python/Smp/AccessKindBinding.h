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

#ifndef PYTHON_SMP_ACCESSKIND_H_
#define PYTHON_SMP_ACCESSKIND_H_

#include <python/ecss_smp.h>
#include <Smp/AccessKind.h>

inline void RegisterAccessKind(const py::module_ &m) {
    py::enum_<::Smp::AccessKind>(m, "AccessKind",
            R"(The Access Kind of a property defines whether it has getter and setter.)")

    .value("AK_ReadWrite", ::Smp::AccessKind::AK_ReadWrite,
            "Read/Write access, i.e. getter and setter.")

    .value("AK_ReadOnly", ::Smp::AccessKind::AK_ReadOnly,
            "Read only access, i.e. only getter method.")

    .value("AK_WriteOnly", ::Smp::AccessKind::AK_WriteOnly,
            "Write only access, i.e. only setter method.");

}

#endif // PYTHON_SMP_ACCESSKIND_H_
