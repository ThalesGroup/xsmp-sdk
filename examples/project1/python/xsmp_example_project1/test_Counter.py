import ecss_smp
import xsmp
import xsmp_example_project1


class TestCounter(xsmp.unittest.TestCase):
    try:
        sim: xsmp_example_project1._test_Counter.Simulator
    except AttributeError:
        pass
    
    def loadAssembly(self, sim: ecss_smp.Smp.ISimulator):
        sim.LoadLibrary("xsmp_example_project1")
        sim.AddModel(sim.CreateInstance(xsmp_example_project1.Example.Counter.uuid, "counter", "", sim))

    def test_Init(self):
        self.assertEqual(self.sim.counter.count, 0, "Initial value of count is 0")

    def test_IncrementCount(self):
        self.sim.counter.IncrementCount()
        self.assertEqual(self.sim.counter.count, 1, "Count incremented")

    def test_ScheduleIncrementCount(self):
        self.sim.GetScheduler().AddSimulationTimeEvent(self.sim.counter.IncrementCount, 0, 1_000_000_000, 5)
        self.sim.Run(seconds=10)
        self.assertEqual(self.sim.counter.count, 6, "Count incremented 6 times")

    def test_ResetCount(self):
        self.sim.counter.count = 10
        self.sim.counter.ResetCount()
        self.assertEqual(self.sim.counter.count, 0, "Count is reseted")
    
    def test_Add(self):
        self.sim.counter.Add(arg=10)
        self.assertEqual(self.sim.counter.count, 10, "Add 10 to count")

