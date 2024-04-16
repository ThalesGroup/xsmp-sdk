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

#ifndef PYTHON_SMP_IENTRYPOINTPUBLISHER_H_
#define PYTHON_SMP_IENTRYPOINTPUBLISHER_H_

#include <Smp/IEntryPointPublisher.h>
#include <python/ecss_smp.h>

inline void RegisterIEntryPointPublisher(const py::module_ &m) {
  py::class_<::Smp::IEntryPointPublisher, ::Smp::IObject>(
      m, "IEntryPointPublisher", py::multiple_inheritance())

      .def("GetEntryPoint", &::Smp::IEntryPointPublisher::GetEntryPoint,
           py::arg("name"), py::return_value_policy::reference,
           R"(Query for an entry point of this component by its name.
The returned entry point may be null if no entry point with the given name could be found.)")

      .doc() =
      R"(Interface of an entry point publisher.
An entry point publisher is a component that publishes entry points.)";
}

#endif // PYTHON_SMP_IENTRYPOINTPUBLISHER_H_
