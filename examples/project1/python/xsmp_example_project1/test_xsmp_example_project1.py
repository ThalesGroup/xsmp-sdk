import ecss_smp
import xsmp
import xsmp_example_project1


class TestXsmpExampleProject1(xsmp.unittest.TestCase):
    try:
        sim: xsmp_example_project1._test_xsmp_example_project1.Simulator
    except AttributeError:
        pass
    
    def loadAssembly(self, sim: ecss_smp.Smp.ISimulator):
        sim.LoadLibrary("xsmp_example_project1")
        sim.AddModel(sim.CreateInstance(xsmp_example_project1.Example.M1.uuid, "name", "", sim))

    def test_Init(self):
        pass
        

    def test_Init2(self):
        pass