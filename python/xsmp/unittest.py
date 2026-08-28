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

import ecss_smp
import os
import sys
import unittest


class TestCase(unittest.TestCase):

    def createSimulator(self, name:str="TestSimulator", description:str="Simulator for unit-tests") -> ecss_smp.Smp.ISimulator:
        return ecss_smp.Smp.createSimulator(library_name="xsmp_simulator", factory_name="createSimulator",
                                                 name=name, description=description)

    def loadServices(self, sim:ecss_smp.Smp.ISimulator) -> None:
        # load mandatory services
        self.loadLogger(sim)
        self.loadTimeKeeper(sim)
        self.loadResolver(sim)
        self.loadEventManager(sim)
        self.loadLinkRegistry(sim)
        self.loadScheduler(sim)
            
    def loadLogger(self, sim:ecss_smp.Smp.ISimulator) -> None:
        sim.LoadLibrary(library_name="xsmp_logger")

    def loadTimeKeeper(self, sim:ecss_smp.Smp.ISimulator) -> None:
        sim.LoadLibrary(library_name="xsmp_time_keeper")

    def loadResolver(self, sim:ecss_smp.Smp.ISimulator) -> None:
        sim.LoadLibrary(library_name="xsmp_resolver")

    def loadEventManager(self, sim:ecss_smp.Smp.ISimulator) -> None:
        sim.LoadLibrary(library_name="xsmp_event_manager")

    def loadLinkRegistry(self, sim:ecss_smp.Smp.ISimulator) -> None:
        sim.LoadLibrary(library_name="xsmp_link_registry")

    def loadScheduler(self, sim:ecss_smp.Smp.ISimulator) -> None:
        sim.LoadLibrary(library_name="xsmp_scheduler")
    
    def loadAssembly(self, sim:ecss_smp.Smp.ISimulator) -> None: ...
    
    def generateTypeHints(self, sim:ecss_smp.Smp.ISimulator):
        """Dump the type hints of the current assembly next to the test module.

        The dump is written next to the test module, as `_<test case>.py`. It
        is named after the test case rather than the module because that is
        what defines the assembly: two test cases in one module build two
        different trees.

        Reference it from the test case behind a `typing.TYPE_CHECKING` guard,
        so that IDEs and type checkers resolve the simulator hierarchy without
        importing the dump at runtime:

            class MyTest(xsmp.unittest.TestCase):
                if typing.TYPE_CHECKING:
                    from ._MyTest import sim

        Import `sim`, not the `Simulator` class: an import binds a value, so
        the class would type `self.sim` as `type[Simulator]` and every instance
        method would then ask for an explicit `self`.

        Override this method with an empty body in a test case that never walks
        the assembly, so that no dump is written beside it.
        """
        module_path = sys.modules[self.__class__.__module__].__file__
        if module_path is None:
            return
        directory_path = os.path.dirname(module_path)
        sim.generate_python_type_hints(
            os.path.join(directory_path,
                         "_" + self.__class__.__name__ + ".py"))

    def setUp(self):
        try:
            # create the simulator
            self.sim = self.createSimulator()
            
            # load mandatory services
            self.loadServices(self.sim)
            
            # load the assembly
            self.loadAssembly(self.sim)
            
            # return the simulator in Connected state
            self.sim.Connect()
            
            self.generateTypeHints(self.sim)
            
        except Exception as err:
            # in case of error, try to deallocate the simulator
            try:
                del self.sim
            except AttributeError:
                pass
            raise err

    def tearDown(self):
        if hasattr(self, "sim"):
            if self.sim.GetState()== ecss_smp.Smp.SimulatorStateKind.SSK_Standby:
                self.sim.Exit()
            # deallocate the simulator
            del self.sim

