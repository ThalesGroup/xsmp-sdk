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

#ifndef PYTHON_SMP_SERVICES_IRESOLVER_H_
#define PYTHON_SMP_SERVICES_IRESOLVER_H_

#include <Smp/Services/IResolver.h>
#include <python/ecss_smp.h>

inline void RegisterIResolver(const py::module_ &m) {

  py::class_<::Smp::Services::IResolver, ::Smp::IService>(
      m, "IResolver", py::multiple_inheritance())

      .def("ResolveAbsolute", &::Smp::Services::IResolver::ResolveAbsolute,
           py::arg("absolute_path"),
           R"(Resolve reference to an object via absolute path.
An absolute path contains the name of either a Model or ervice, but not the name of the simulator, although the simulator itself is the top-level object.
This allows keeping names as short as possible, and avoids a dependency on the name of the simulator itself.)")

      .def("ResolveRelative", &::Smp::Services::IResolver::ResolveRelative,
           py::arg("relative_path"), py::arg("sender"),
           "Resolve reference to an object via relative path.")

      .doc() = "This interface gives access to the Resolver Service.";
}

#endif // PYTHON_SMP_SERVICES_IRESOLVER_H_
