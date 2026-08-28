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

#ifndef PYTHON_SMP_REQUESTTYPE_H_
#define PYTHON_SMP_REQUESTTYPE_H_

#include <Smp/RequestType.h>
#include <python/ecss_smp.h>

inline void RegisterRequestType(const py::module_ &m) {
  py::enum_<::Smp::RequestType>(
      m, "RequestType",
      "Tells what a request object stands for: an operation to invoke, or the "
      "getter or the setter of a property.")

      .value("RT_Invoke", ::Smp::RequestType::RT_Invoke,
             "Request represents an Operation.")

      .value("RT_Get", ::Smp::RequestType::RT_Get,
             "Request represents a Property getter.")

      .value("RT_Set", ::Smp::RequestType::RT_Set,
             "Request represents a Property setter.");
}

#endif // PYTHON_SMP_REQUESTTYPE_H_
