import ecss_smp
import xsmp
import xsmp_example_project1


class TestCounterManager(xsmp.unittest.TestCase):
    try:
        sim: xsmp_example_project1._test_CounterManager.Simulator
    except AttributeError:
        pass
    
    def loadAssembly(self, sim: ecss_smp.Smp.ISimulator):
        sim.LoadLibrary("xsmp_example_project1")
        sim.AddModel(sim.CreateInstance(xsmp_example_project1.Example.CounterManager.uuid, "manager", '', sim))
        sim.manager._counters += sim.CreateInstance(xsmp_example_project1.Example.Counter.uuid, "counter1", "", sim.manager)
        sim.manager._counters += sim.CreateInstance(xsmp_example_project1.Example.Counter.uuid, "counter2", "", sim.manager)
        sim.manager._counters += sim.CreateInstance(xsmp_example_project1.Example.Counter.uuid, "counter3", "", sim.manager)
        sim.manager._counters += sim.CreateInstance(xsmp_example_project1.Example.Counter.uuid, "counter4", "", sim.manager)

    def test_Init(self):
        self.assertEqual(self.sim.manager.sum, 0, "Initial value of sum is 0")
        self.assertEqual(self.sim.manager.average, 0, "Initial value of average is 0")

    def test_Sum(self):
        self.sim.manager.counter1.count = 1
        self.assertEqual(self.sim.manager.sum, 1)
        self.sim.manager.counter2.count = 2
        self.assertEqual(self.sim.manager.sum, 3)
        self.sim.manager.counter3.count = 4
        self.assertEqual(self.sim.manager.sum, 7)
        self.sim.manager.counter4.count = 8
        self.assertEqual(self.sim.manager.sum, 15)
        
    def test_Average(self):
        self.sim.manager.counter1.count = 2
        self.sim.manager.counter2.count = 2
        self.sim.manager.counter3.count = 4
        self.sim.manager.counter4.count = 4
        self.assertEqual(self.sim.manager.average, 3)


    def test_Count(self):
        self.sim.manager.count = 8
        self.assertEqual(self.sim.manager.counter1.count, 8)
        self.assertEqual(self.sim.manager.counter2.count, 8)
        self.assertEqual(self.sim.manager.counter3.count, 8)
        self.assertEqual(self.sim.manager.counter4.count, 8)
