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

#ifndef PYTHON_SMP_LIBRARYLOADINGFLAG_H_
#define PYTHON_SMP_LIBRARYLOADINGFLAG_H_

#include <Smp/LibraryLoadingFlag.h>
#include <python/ecss_smp.h>

inline void RegisterLibraryLoadingFlag(const py::module_ &m) {
  py::enum_<::Smp::LibraryLoadingFlag>(
      m, "LibraryLoadingFlag",
      "Describes whether the symbols a library defines are made available to "
      "the libraries loaded afterwards.")

      .value("LLF_Auto", ::Smp::LibraryLoadingFlag::LLF_Auto,
             "The library loading option is selected by the simulation "
             "environment implementation.")

      .value("LLF_Local", ::Smp::LibraryLoadingFlag::LLF_Local,
             "Symbols defined in this library are not made available to "
             "resolve references in subsequently loaded libraries.")

      .value("LLF_Global", ::Smp::LibraryLoadingFlag::LLF_Global,
             "The symbols defined by this library are made available for "
             "symbol resolution of subsequently loaded libraries.");
}

#endif // PYTHON_SMP_LIBRARYLOADINGFLAG_H_
