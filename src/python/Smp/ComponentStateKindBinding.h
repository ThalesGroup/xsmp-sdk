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

#ifndef PYTHON_SMP_COMPONENTSTATEKIND_H_
#define PYTHON_SMP_COMPONENTSTATEKIND_H_

#include <python/ecss_smp.h>
#include <Smp/ComponentStateKind.h>

inline void RegisterComponentStateKind(const py::module_ &m) {
    py::enum_<::Smp::ComponentStateKind>(m, "ComponentStateKind")

    .value("CSK_Created", ::Smp::ComponentStateKind::CSK_Created,
            R"(The Created state is the initial state of a component. Component creation is done by an external mechanism, e.g. by factories.
This state is entered automatically after the component has been created.
This state is left via the Publish() state transition.)")

    .value("CSK_Publishing", ::Smp::ComponentStateKind::CSK_Publishing,
            R"(In Publishing state, the component is allowed to publish features. 
This includes publication of fields, operations and properties. In addition, the component is allowed to create other components.
This state is entered via the Publish() state transition.
This state is left via the Configure() state transition.)")

    .value("CSK_Configured", ::Smp::ComponentStateKind::CSK_Configured,
            R"(In Configured state, the component has been fully configured. This configuration may be done by external components, or internally by the component itself, e.g. by reading data from an external source.
This state is entered via the Configure() state transition.
This state is left via the Connect() state transition.)")

    .value("CSK_Connected", ::Smp::ComponentStateKind::CSK_Connected,
            R"(Connected state, the component is connected to the simulator. In this state, neither publication nor creation of other components is allowed anymore. Configuration performed via loading of SMDL configuration file and/or calling of initialisation entry point are performed in this state.
This state is entered via the Connect() state transition.
This state is left via the Disconnect() state transition or on simulation termination.)")

    .value("CSK_Disconnected", ::Smp::ComponentStateKind::CSK_Disconnected,
            R"(In Disconnected state, the component is disconnected from the simulator, and all references to it are deleted, so that it can be deleted.
This state is entered via the Disconnect() state transition.
This is the final state of a component, and only left on deletion.)");
}

#endif // PYTHON_SMP_COMPONENTSTATEKIND_H_
