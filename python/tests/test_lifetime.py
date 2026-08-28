# Copyright 2023 THALES ALENIA SPACE FRANCE. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import gc

import ecss_smp
import xsmp

# the objects kept here are released when the interpreter shuts down: they must
# keep the simulator that owns them alive, otherwise the C++ objects are
# destroyed first and their wrappers are left dangling
_kept = []


class TestLifetime(xsmp.unittest.TestCase):

    def generateTypeHints(self, sim):
        # this test never walks the assembly, so it needs no dump beside it
        pass

    def testKeptAfterTheSimulatorIsReleased(self):
        sim = self.createSimulator()
        self.loadServices(sim)

        logger = sim.GetLogger()
        scheduler = sim.GetResolver().ResolveAbsolute("/XsmpScheduler")
        _kept.append(logger)
        _kept.append(scheduler)

        del sim
        gc.collect()

        self.assertEqual(logger.GetName(), "XsmpLogger")
        self.assertEqual(scheduler.GetName(), "XsmpScheduler")

    def testResolvedTwiceIsTheSameObject(self):
        resolver = self.sim.GetResolver()
        self.assertIs(resolver.ResolveAbsolute("/XsmpLogger"),
                      resolver.ResolveAbsolute("/XsmpLogger"))
