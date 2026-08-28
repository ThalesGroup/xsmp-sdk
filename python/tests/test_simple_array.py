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

"""Slicing a simple array field from Python."""

import ecss_smp
import xsmp
import xsmp_tests


class TestSimpleArrayField(xsmp.unittest.TestCase):

    def loadAssembly(self, sim: ecss_smp.Smp.ISimulator):
        sim.LoadLibrary("xsmp_tests")
        sim._Models += sim.CreateInstance(
            uuid=xsmp_tests.Xsmp.Tests.ModelWithSimpleArrayFields.uuid,
            name="arrays", parent=sim)

    def testItem(self):
        field = self.sim.arrays.integer1
        self.assertEqual(len(field), 3)
        field[0] = 7
        self.assertEqual(field[0], 7)
        self.assertEqual(field[-1], field[2])
        with self.assertRaises(IndexError):
            field[3]

    def testSlice(self):
        field = self.sim.arrays.integer1
        # a full slice is read and written through GetValues()/SetValues()
        field[:] = [10, 20, 30]
        self.assertEqual(field[:], [10, 20, 30])
        self.assertEqual(field[1:3], [20, 30])
        self.assertEqual(field[-2:], [20, 30])
        self.assertEqual(field[1:1], [])

        # a partial slice reads and writes the items it covers
        field[0:2] = [1, 2]
        self.assertEqual(field[:], [1, 2, 30])
        self.assertEqual(field[::2], [1, 30])
        field[::2] = [4, 5]
        self.assertEqual(field[:], [4, 2, 5])

    def testSliceLengthMismatch(self):
        field = self.sim.arrays.integer1
        with self.assertRaises(ValueError):
            field[0:2] = [1, 2, 3]
