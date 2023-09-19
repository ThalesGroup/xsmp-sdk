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

import sys


class ModelWithPropertiesTest(xsmp.unittest.TestCase):
    try:
        sim: tests._test_ModelWithProperties.Simulator
    except AttributeError:
        pass
    
    def loadAssembly(self, sim:ecss_smp.Smp.ISimulator):
        sim.LoadLibrary("xsmp_tests")
        # create an Instance of M1 in Models
        sim._Models += sim.CreateInstance(uuid=xsmp_tests.Xsmp.Tests.ModelWithProperties.uuid, name="test", parent=sim)

    def testIntProperty(self):
        # test getter
        self.sim.test.int_field = 5
        self.assertEqual(self.sim.test.int_property, 5)
        self.assertEqual(self.sim.test.readonly_int_property, 5)
        self.assertEqual(self.sim.test.int_property.GetValue(), 5)
        if sys.platform.startswith("darwin"):
            with self.assertRaises(RuntimeError):
                self.sim.test.writeonly_int_property.GetValue()
        else: 
            with self.assertRaises(ecss_smp.Smp.Exception):
                self.sim.test.writeonly_int_property.GetValue()
        
        # test setter
        self.sim.test.int_property = 10
        self.assertEqual(self.sim.test.int_field, 10)
        self.sim.test.int_property.SetValue(15)
        self.assertEqual(self.sim.test.int_field, 15)
        self.sim.test.writeonly_int_property = 20
        self.assertEqual(self.sim.test.int_field, 20)
        if sys.platform.startswith("darwin"):
            with self.assertRaises(RuntimeError):
                self.sim.test.readonly_int_property.SetValue(25)
        else:
            with self.assertRaises(ecss_smp.Smp.Exception):
                self.sim.test.readonly_int_property.SetValue(25)

    def testStaticIntProperty(self):
        # test getter
        self.sim.test.static_int_field = 5
        self.assertEqual(self.sim.test.static_int_property, 5)
        self.assertEqual(self.sim.test.readonly_static_int_property, 5)
        self.assertEqual(self.sim.test.static_int_property.GetValue(), 5)

        if sys.platform.startswith("darwin"):
            with self.assertRaises(RuntimeError):
                self.sim.test.writeonly_static_int_property.GetValue()
        else:
            with self.assertRaises(ecss_smp.Smp.Exception):
                self.sim.test.writeonly_static_int_property.GetValue()
        
        # test setter
        self.sim.test.static_int_property = 10
        self.assertEqual(self.sim.test.static_int_field, 10)
        self.sim.test.static_int_property.SetValue(15)
        self.assertEqual(self.sim.test.static_int_field, 15)
        self.sim.test.writeonly_static_int_property = 20
        self.assertEqual(self.sim.test.static_int_field, 20)
        if sys.platform.startswith("darwin"):
            with self.assertRaises(RuntimeError):
                self.sim.test.readonly_static_int_property.SetValue(25)
        else:
            with self.assertRaises(ecss_smp.Smp.Exception):
                self.sim.test.readonly_static_int_property.SetValue(25)

    def testUuidProperty(self):

        with self.assertRaises(AttributeError):
            self.sim.test.uuid_property.GetValue()


if __name__ == '__main__':
    unittest.main()
