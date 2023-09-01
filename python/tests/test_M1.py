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


import unittest
import ecss_smp
import xsmp_tests
import xsmp


import tests


def create_assembly(sim:ecss_smp.Smp.ISimulator):
    toto = sim.CreateInstance(uuid=xsmp_tests.Xsmp.Tests.ModelWithSimpleFields.uuid, name="toto", description="toto", parent=sim)
    sim._Models += toto
    toto._subModels += sim.CreateInstance(uuid=xsmp_tests.Xsmp.Tests.ModelWithSimpleFields.uuid, name="s1", description="s1", parent=sim.toto)
    toto._subModels += sim.CreateInstance(uuid=xsmp_tests.Xsmp.Tests.ModelWithSimpleFields.uuid, name="s2", description="s2", parent=toto)
    toto._subModels += sim.CreateInstance(uuid=xsmp_tests.Xsmp.Tests.ModelWithSimpleFields.uuid, name="s3", description="s3", parent=toto)

#help(xsmp.unittest)
class M1Test(xsmp.unittest.TestCase):
    try:
        sim: tests._test_M1.Simulator
    except AttributeError:
        pass
    
    def loadAssembly(self, sim:ecss_smp.Smp.ISimulator):
        sim.LoadLibrary("xsmp_tests")
        # create an Instance of M1 in Models
        sim._Models += sim.CreateInstance(uuid=xsmp_tests.Xsmp.Tests.ModelWithSimpleFields.uuid, name="test", parent=sim)
        create_assembly(sim)

    def testSimulator(self):
        sim = self.sim
        self.assertEqual(sim.XsmpLogger, sim.GetLogger())
        self.assertEqual(sim.XsmpEventManager, sim.GetEventManager())
        self.assertEqual(sim.XsmpLinkRegistry, sim.GetLinkRegistry())
        self.assertEqual(sim.XsmpResolver, sim.GetResolver())
        self.assertEqual(sim.XsmpScheduler, sim.GetScheduler())
        self.assertEqual(sim.XsmpTimeKeeper, sim.GetTimeKeeper())

    def testMultiSimulator(self):
        # create a 2nd simulator
        sim2 = self.createSimulator()
        self.loadServices(sim2)
        self.loadAssembly(sim2)
        sim2.Connect()
        del sim2
        del self.sim
        
    def testLogger(self):
        logger = self.sim.GetLogger()
        self.assertEqual(logger, self.sim.XsmpLogger)
        self.assertEqual(ecss_smp.Smp.Services.ILogger.LMK_Debug, logger.LMK_Debug)
        self.assertEqual(ecss_smp.Smp.Services.ILogger.LMK_Event, logger.LMK_Event)
        self.assertEqual(ecss_smp.Smp.Services.ILogger.LMK_Information, logger.LMK_Information)
        self.assertEqual(ecss_smp.Smp.Services.ILogger.LMK_Warning, logger.LMK_Warning)
        self.assertEqual(ecss_smp.Smp.Services.ILogger.LMK_Error, logger.LMK_Error)
        
        self.assertEqual(ecss_smp.Smp.Services.ILogger.LMK_DebugName, logger.LMK_DebugName)
        self.assertEqual(ecss_smp.Smp.Services.ILogger.LMK_EventName, logger.LMK_EventName)
        self.assertEqual(ecss_smp.Smp.Services.ILogger.LMK_InformationName, logger.LMK_InformationName)
        self.assertEqual(ecss_smp.Smp.Services.ILogger.LMK_WarningName, logger.LMK_WarningName)
        self.assertEqual(ecss_smp.Smp.Services.ILogger.LMK_ErrorName, logger.LMK_ErrorName)
        
        self.assertEqual(logger.QueryLogMessageKind(logger.LMK_DebugName), logger.LMK_Debug)
        self.assertEqual(logger.QueryLogMessageKind(logger.LMK_EventName), logger.LMK_Event)
        self.assertEqual(logger.QueryLogMessageKind(logger.LMK_InformationName), logger.LMK_Information)
        self.assertEqual(logger.QueryLogMessageKind(logger.LMK_WarningName), logger.LMK_Warning)
        self.assertEqual(logger.QueryLogMessageKind(logger.LMK_ErrorName), logger.LMK_Error)
        
        user_msg = logger.QueryLogMessageKind("user_msg")
        self.assertEqual(logger.QueryLogMessageKind("user_msg"), user_msg)
        
    def testTimeKeeper(self):
        time_keeper = self.sim.GetTimeKeeper()
        self.assertEqual(time_keeper, self.sim.XsmpTimeKeeper)

    def testEvents(self):
        test = self.sim.test

        self.sim.test.eso += test.esi
        
        test.eso -= test.esi
        
        test.esi += test.eso
        with self.assertRaises(RuntimeError):
            test.esi += test.eso
            
        test.esi -= test.eso
        with self.assertRaises(RuntimeError):
            test.esi -= test.eso

    def testSimpleField(self):
        sim = self.sim
        test = sim.test
        
        # Boolean conversion
        test.integer1 = 0
        sim.toto.s1.integer1 = 10

        self.assertFalse(test.integer1)
        self.assertFalse(test["integer1"])
        
        self.assertFalse(test["integer1"].IsInput())
        
        test["integer1"] = 5
        self.assertTrue(test.integer1)
        
        
        test.integer1Transient = test.integer1
        self.assertEqual(test.integer1Transient , 5)
        self.assertTrue(test.integer1 == test.integer1Transient)
        
        # comparison operators
        self.assertTrue(test.integer1 == 5)
        self.assertTrue(test.integer1 != 4)
        self.assertTrue(test.integer1 < 6)
        self.assertTrue(test.integer1 <= 5)
        self.assertTrue(test.integer1 > 4)
        self.assertTrue(test.integer1 >= 5)
        self.assertTrue(5 == test.integer1)
        self.assertTrue(4 != test.integer1)
        self.assertTrue(6 > test.integer1)
        self.assertTrue(5 >= test.integer1)
        self.assertTrue(4 < test.integer1)
        self.assertTrue(5 <= test.integer1)
        
        # arithmetics operations
        self.assertEqual(test.integer1 + 1, 6)
        self.assertEqual(1 + test.integer1, 6)
        
        self.assertEqual(test.integer1 - 1, 4)
        self.assertEqual(1 - test.integer1, -4)
        
        self.assertEqual(test.integer1 * 1, 5)
        self.assertEqual(1 * test.integer1, 5)
        
        self.assertEqual(test.integer1 / 1, 5)
        self.assertEqual(1 / test.integer1, 0.2)
        
        self.assertEqual(5, test.integer1)
        self.assertEqual(test.integer1, test.integer1)
        
        self.assertNotEqual(test.integer1, 6.3)
        self.assertNotEqual(6, test.integer1)
        
        test.integer1 += 1
        self.assertEqual(test.integer1 , 6)
        test.integer1 -= 1
        self.assertEqual(test.integer1 , 5)
        test.integer1 *= 2
        self.assertEqual(test.integer1 , 10)
        test.integer1 -= test.integer1
        self.assertEqual(test.integer1 , 0)
        
        # test.integer1 /= 2
        # self.assertEqual(test.integer1 , 5)
        
        test.float1 = 5.4
        self.assertEqual(test.float1, 5.4)
        
        v = test.string1
        self.assertEqual(v, "")
        test.string1 = "01234567890"
        self.assertEqual(test.string1, "0123456789")
        
        test.enum1 = 2


if __name__ == '__main__':
    unittest.main()
