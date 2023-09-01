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
#include <Smp/IEntryPoint.h>
#include <string>
#include <typeinfo>

inline void RegisterIEntryPoint(const py::module_ &m) {
    py::class_<::Smp::IEntryPoint, ::Smp::IObject>(m, "IEntryPoint",
            py::multiple_inheritance())

    .def("__call__", &::Smp::IEntryPoint::Execute, "Call the EntryPoint.")

    .def("Execute", &::Smp::IEntryPoint::Execute,
            "This method shall be called when an associated event is emitted.")

    .doc() =
            R"(Interface of an entry point.
This interface provides a notification method (event handler) that can be called e.g. by the Scheduler or Event Manager when an event is emitted.)";
}

