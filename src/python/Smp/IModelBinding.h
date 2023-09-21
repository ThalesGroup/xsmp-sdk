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

#ifndef PYTHON_SMP_IMODEL_H_
#define PYTHON_SMP_IMODEL_H_

#include <python/ecss_smp.h>

#include <Smp/IModel.h>

inline void RegisterIModel(const py::module_ &m) {
    py::class_<::Smp::IModel, ::Smp::IComponent>(m, "IModel",
            py::multiple_inheritance())

    .doc() = R"(Interface for a model.)";
}

#endif // PYTHON_SMP_IMODEL_H_