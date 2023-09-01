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

#ifndef XSMP_TESTHELPER_H_
#define XSMP_TESTHELPER_H_

#include <Xsmp/Simulator.h>

#include <Xsmp/LibraryHelper.h>

namespace Xsmp {
namespace TestHelper {

inline void InitializeSimulator(::Xsmp::Simulator &sim) {

    sim.LoadLibrary("xsmp_logger");
    sim.LoadLibrary("xsmp_time_keeper");
    sim.LoadLibrary("xsmp_resolver");
    sim.LoadLibrary("xsmp_event_manager");
    sim.LoadLibrary("xsmp_link_registry");
    sim.LoadLibrary("xsmp_scheduler");
}

} // namespace TestHelper
} // namespace Xsmp
#endif /* XSMP_TESTHELPER_H_ */
