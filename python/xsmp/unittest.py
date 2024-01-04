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
        directory_path, file_name = os.path.split(sys.modules[self.__class__.__module__].__file__)
        sim.generate_python_type_hints(os.path.join(directory_path, "_" + file_name))
    
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

