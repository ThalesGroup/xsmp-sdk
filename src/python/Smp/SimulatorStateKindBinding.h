// Copyright 2023 THALES ALENIA SPACE FRANCE. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef PYTHON_SMP_SIMULATORSTATEKIND_H_
#define PYTHON_SMP_SIMULATORSTATEKIND_H_

#include <Smp/SimulatorStateKind.h>
#include <python/ecss_smp.h>

inline void RegisterSimulatorStateKind(const py::module_ &m) {
  py::enum_<::Smp::SimulatorStateKind>(m, "SimulatorStateKind")
      // SSK_Building
      .value(
          "SSK_Building", ::Smp::SimulatorStateKind::SSK_Building,
          R"(In Building state, the component hierarchy is created. This is done by an external component, not by the simulator.
This state is entered automatically after the simulation environment has performed its initialisation.
In this state, Publish() and Configure() can be called any time to call the corresponding Publish() and Configure() operations of each component.
This state is left with the Connect() state transition method.)")
      // SSK_Connecting
      .value(
          "SSK_Connecting", ::Smp::SimulatorStateKind::SSK_Connecting,
          R"(In Connecting state, the simulation environment traverses the component hierarchy and calls the Connect() method of each component.
This state is entered using the Connect() state transition.
After connecting all components to the simulator, an automatic state transition to the Initialising state is performed.)")

      // SSK_Initialising
      .value(
          "SSK_Initialising", ::Smp::SimulatorStateKind::SSK_Initialising,
          R"(In Initialising state, the simulation environment executes all initialisation entry points in the order they have been added to the simulator using the AddInitEntryPoint() method.
This state is either entered automatically after the simulation environment has connected all models to the simulator, or manually from Standby state using the Initialise() state transition.
After calling all initialisation entry points, an automatic state transition to the Standby state is performed.)")

      // SSK_Standby
      .value(
          "SSK_Standby", ::Smp::SimulatorStateKind::SSK_Standby,
          R"(In Standby state, the simulation environment (namely the Time Keeper Service) does not progress simulation time. Only entry points registered relative to Zulu time are executed.
This state is entered automatically from the Initialising, Storing, and Restoring states, or manually from the Executing state using the Hold() state transition.
This state is left with one of the Run(), Store(), Restore(), Initialise(), Reconnect() or Exit() state transitions.)")

      // SSK_Executing
      .value(
          "SSK_Executing", ::Smp::SimulatorStateKind::SSK_Executing,
          R"(In Executing state, the simulation environment (namely the Time Keeper Service) does progress simulation time. Entry points registered with any of the available time kinds are executed.
This state is entered using the Run() state transition.
This state is left using the Hold() state transition.)")

      // SSK_Storing
      .value(
          "SSK_Storing", ::Smp::SimulatorStateKind::SSK_Storing,
          R"(In Storing state, the simulation environment first stores the values of all fields published with the State attribute to storage (typically a file). Afterwards, the Store() method of all components (Models and Services) implementing the optional IPersist
interface is called, to allow custom storing of additional information. While in this state, fields published with the State attribute must not be modified by the models, to ensure that a consistent set of field values is stored.
This state is entered using the Store() state transition.
After storing the simulator state, an automatic state transition to the Standby state is performed.)")

      // SSK_Restoring
      .value(
          "SSK_Restoring", ::Smp::SimulatorStateKind::SSK_Restoring,
          R"(In Restoring state, the simulation environment first restores the values of all fields published with the State attribute from storage. Afterwards, the Restore() method of all components implementing the optional IPersist interface is called, to allow custom restoring of additional information. While in this state, fields published with the State attribute must not be modified by the models, to ensure that a consistent set of field values is restored.
This state is entered using the Restore() state transition.
After restoring the simulator state, an automatic state transition 
to the Standby state is performed.)")

      // SSK_Reconnecting
      .value(
          "SSK_Reconnecting", ::Smp::SimulatorStateKind::SSK_Reconnecting,
          R"(In Reconnecting state, the simulation environment makes sure that models that have been added to the simulator after leaving the Building state are properly published, configured and connected.
This state is entered using the Reconnect() state transition.
After connecting all new models, an automatic state transition to the Standby state is performed.)")

      // SSK_Exiting
      .value(
          "SSK_Exiting", ::Smp::SimulatorStateKind::SSK_Exiting,
          R"(In Exiting state, the simulation environment is properly terminating a running simulation.
This state is entered using the Exit() state transition. After exiting, the simulator is in an undefined state.)")
      // SSK_Aborting
      .value(
          "SSK_Aborting", ::Smp::SimulatorStateKind::SSK_Aborting,
          R"(In this state, the simulation environment performs an abnormal simulation shut-down.
This state is entered using the Abort() state transition. After aborting, the simulator is in an undefined state.)");
}

#endif // PYTHON_SMP_SIMULATORSTATEKIND_H_
