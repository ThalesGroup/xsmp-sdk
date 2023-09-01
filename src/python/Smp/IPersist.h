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

#include <python/ecss_smp.h>
#include <pybind11/pybind11.h>
#include <Smp/IPersist.h>

inline void RegisterIPersist(const py::module_ &m) {
    py::class_<::Smp::IPersist, ::Smp::IObject>(m, "IPersist",
            py::multiple_inheritance())

    .doc() =
            R"(Interface of a self-persisting object that provides operations to allow for storing and restoring its state.
An object may implement this interface if it wants to have control over storing and restoring of its state. This is an optional interface which needs to be implemented by objects with self-persistence only.)";
}

