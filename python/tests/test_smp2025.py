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

"""The interfaces and the types SMP 2025 added, as seen from Python."""

import ecss_smp
import xsmp
import xsmp_tests


class TestSmp2025(xsmp.unittest.TestCase):

    def loadAssembly(self, sim: ecss_smp.Smp.ISimulator):
        sim.LoadLibrary("xsmp_tests",
                        flag=ecss_smp.Smp.LibraryLoadingFlag.LLF_Auto)
        sim._Models += sim.CreateInstance(
            uuid=xsmp_tests.Xsmp.Tests.ModelWithSimpleFields.uuid,
            name="model", parent=sim)

    def testNewEnums(self):
        self.assertEqual(
            [*ecss_smp.Smp.LibraryLoadingFlag.__members__],
            ["LLF_Auto", "LLF_Local", "LLF_Global"])
        self.assertEqual(
            [*ecss_smp.Smp.RequestType.__members__],
            ["RT_Invoke", "RT_Get", "RT_Set"])

    def testNewExceptions(self):
        # a library that is not a package cannot report its SMP version
        with self.assertRaises(ecss_smp.Smp.InvalidSmpVersion):
            self.sim.LoadLibrary("xsmp_simulator")

    def testGetChild(self):
        model = self.sim.model
        self.assertEqual(model.GetChild("boolean"), model.boolean)
        # a structure field answers with its sub-fields
        self.assertEqual(model.struct1.GetChild("integer1"),
                         model.struct1.integer1)
        self.assertIsNone(model.GetChild("absent"))
        # the simulator answers with its containers and its models
        self.assertEqual(self.sim.GetChild("model"), model)

    def testSimpleValues(self):
        model = self.sim.model
        model.SetSimpleValue("integer1", 42)
        self.assertEqual(model.GetSimpleValue("integer1"), 42)
        # a sub-field of a structure is reached by its full name
        model.SetSimpleValue("struct1.integer1", 7)
        self.assertEqual(model.GetSimpleValue("struct1.integer1"), 7)

    def testCollectionIsEmpty(self):
        self.assertFalse(self.sim.model.GetFields().empty())

    def testOutputField(self):
        output = self.sim.model.booleanOutput
        self.assertIsInstance(output, ecss_smp.Smp.IOutputField)
        # XSMP pushes explicitly, never on assignment
        self.assertFalse(output.IsAutomatic())
        self.assertTrue(output.GetInputFields().empty())

        target = self.sim.model.booleanInput
        output.Connect(target)
        self.assertEqual(output.GetInputFields().size(), 1)
        output.Push()
        output.Disconnect(target)
        self.assertTrue(output.GetInputFields().empty())
        with self.assertRaises(ecss_smp.Smp.FieldNotConnected):
            output.Disconnect(target)
