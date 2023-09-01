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

#include <gtest/gtest.h>
#include <Smp/Services/EntryPointAlreadySubscribed.h>
#include <Smp/Services/EntryPointNotSubscribed.h>
#include <Smp/Services/EventId.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/InvalidEventId.h>
#include <Smp/Services/InvalidEventName.h>
#include <Xsmp/Component.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/EntryPointPublisher.h>
#include <Xsmp/Simulator.h>
#include <Xsmp/TestHelper.h>
#include <initializer_list>
#include <map>
#include <string>
#include <vector>

namespace Xsmp::Services {
TEST(XsmpResolverTest, ResolveAbsolute) {

    Simulator sim;
    ::Xsmp::TestHelper::InitializeSimulator(sim);

   EXPECT_EQ( sim.GetResolver()->ResolveAbsolute("/"), &sim);
   EXPECT_EQ( sim.GetResolver()->ResolveAbsolute("/XsmpResolver"), sim.GetResolver());


   EXPECT_FALSE( sim.GetResolver()->ResolveAbsolute(nullptr));
   EXPECT_FALSE( sim.GetResolver()->ResolveAbsolute(""));
   EXPECT_FALSE( sim.GetResolver()->ResolveAbsolute("."));
}
TEST(XsmpResolverTest, ResolveRelative) {

    Simulator sim;
    ::Xsmp::TestHelper::InitializeSimulator(sim);

   EXPECT_EQ( sim.GetResolver()->ResolveRelative("..", sim.GetResolver()), &sim);
   EXPECT_EQ( sim.GetResolver()->ResolveRelative(".", sim.GetResolver()), sim.GetResolver());

   EXPECT_FALSE( sim.GetResolver()->ResolveRelative(nullptr, sim.GetResolver()));
   EXPECT_FALSE( sim.GetResolver()->ResolveRelative("/", sim.GetResolver()));
}

} // namespace Xsmp::Services
