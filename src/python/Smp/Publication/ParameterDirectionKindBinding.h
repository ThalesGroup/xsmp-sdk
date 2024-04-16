// Copyright 2024 THALES ALENIA SPACE FRANCE. All rights reserved.
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

#ifndef PYTHON_SMP_PUBLICATION_PARAMETERDIRECTIONKINDKIND_H_
#define PYTHON_SMP_PUBLICATION_PARAMETERDIRECTIONKINDKIND_H_

#include <Smp/Publication/ParameterDirectionKind.h>
#include <python/ecss_smp.h>

inline void RegisterParameterDirectionKind(const py::module_ &m) {
  py::enum_<::Smp::Publication::ParameterDirectionKind>(
      m, "ParameterDirectionKind")

      .value(
          "PDK_In", ::Smp::Publication::ParameterDirectionKind::PDK_In,
          "The parameter is read-only to the operation, i.e. its value must be "
          "specified on call, and cannot be changed inside the operation.")

      .value("PDK_Out", ::Smp::Publication::ParameterDirectionKind::PDK_Out,
             "The parameter is write-only to the operation, i.e. its value is "
             "unspecified on call, and must be set by the operation.")

      .value("PDK_InOut", ::Smp::Publication::ParameterDirectionKind::PDK_InOut,
             "The parameter must be specified on call, and may be changed by "
             "the operation.")

      .value("PDK_Return",
             ::Smp::Publication::ParameterDirectionKind::PDK_Return,
             "The parameter represents the operation's return value.");
}

#endif // PYTHON_SMP_PUBLICATION_PARAMETERDIRECTIONKINDKIND_H_
