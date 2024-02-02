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

#include <Smp/AccessKind.h>
#include <Smp/ComponentStateKind.h>
#include <Smp/Publication/ParameterDirectionKind.h>
#include <Smp/Services/TimeKind.h>
#include <Smp/SimulatorStateKind.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Publication/Type.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <limits>

namespace Xsmp::Publication {

TypeRegistry::TypeRegistry(::Smp::IObject *parent) :
        _parent(parent) {

    AddType<PrimitiveType>("Char8", "8 bit character", this,
            ::Smp::PrimitiveTypeKind::PTK_Char8, ::Smp::Uuids::Uuid_Char8);
    AddType<PrimitiveType>("String8", "8 bit character string", this,
            ::Smp::PrimitiveTypeKind::PTK_String8, ::Smp::Uuids::Uuid_String8);
    AddType<PrimitiveType>("Bool", "boolean with true or false", this,
            ::Smp::PrimitiveTypeKind::PTK_Bool, ::Smp::Uuids::Uuid_Bool);
    AddType<PrimitiveType>("Int8", "8 bit signed integer", this,
            ::Smp::PrimitiveTypeKind::PTK_Int8, ::Smp::Uuids::Uuid_Int8);
    AddType<PrimitiveType>("Int16", "16 bit signed integer", this,
            ::Smp::PrimitiveTypeKind::PTK_Int16, ::Smp::Uuids::Uuid_Int16);
    AddType<PrimitiveType>("Int32", "32 bit signed integer", this,
            ::Smp::PrimitiveTypeKind::PTK_Int32, ::Smp::Uuids::Uuid_Int32);
    AddType<PrimitiveType>("Int64", "64 bit signed integer", this,
            ::Smp::PrimitiveTypeKind::PTK_Int64, ::Smp::Uuids::Uuid_Int64);
    AddType<PrimitiveType>("UInt8", "8 bit unsigned integer", this,
            ::Smp::PrimitiveTypeKind::PTK_UInt8, ::Smp::Uuids::Uuid_UInt8);
    AddType<PrimitiveType>("UInt16", "16 bit unsigned integer", this,
            ::Smp::PrimitiveTypeKind::PTK_UInt16, ::Smp::Uuids::Uuid_UInt16);
    AddType<PrimitiveType>("UInt32", "32 bit unsigned integer", this,
            ::Smp::PrimitiveTypeKind::PTK_UInt32, ::Smp::Uuids::Uuid_UInt32);
    AddType<PrimitiveType>("UInt64", "64 bit unsigned integer", this,
            ::Smp::PrimitiveTypeKind::PTK_UInt64, ::Smp::Uuids::Uuid_UInt64);
    AddType<PrimitiveType>("Float32", "32 bit single-precision float", this,
            ::Smp::PrimitiveTypeKind::PTK_Float32, ::Smp::Uuids::Uuid_Float32);
    AddType<PrimitiveType>("Float64", "64 bit double-precision float", this,
            ::Smp::PrimitiveTypeKind::PTK_Float64, ::Smp::Uuids::Uuid_Float64);
    AddType<PrimitiveType>("Duration", "duration in nanoseconds", this,
            ::Smp::PrimitiveTypeKind::PTK_Duration,
            ::Smp::Uuids::Uuid_Duration);
    AddType<PrimitiveType>("DateTime",
            "point in time in nanoseconds relative to MJD 2000+0.5", this,
            ::Smp::PrimitiveTypeKind::PTK_DateTime,
            ::Smp::Uuids::Uuid_DateTime);

    auto *primitiveTypeKind = AddEnumerationType("PrimitiveTypeKind", // name
            "This is an enumeration of the available primitive types.", // description
            ::Smp::Uuids::Uuid_PrimitiveTypeKind, // UUID
            sizeof(::Smp::PrimitiveTypeKind));

    // Register the Literals of the Enumeration
    primitiveTypeKind->AddLiteral("PTK_None", "No type, e.g. for void.", 0);
    primitiveTypeKind->AddLiteral("PTK_Char8", "8 bit character type.", 1);
    primitiveTypeKind->AddLiteral("PTK_Bool", "Boolean with true and false.",
            2);
    primitiveTypeKind->AddLiteral("PTK_Int8", "8 bit signed integer type.", 3);
    primitiveTypeKind->AddLiteral("PTK_UInt8", "8 bit unsigned integer type.",
            4);
    primitiveTypeKind->AddLiteral("PTK_Int16", "16 bit signed integer type.",
            5);
    primitiveTypeKind->AddLiteral("PTK_UInt16", "16 bit unsigned integer type.",
            6);
    primitiveTypeKind->AddLiteral("PTK_Int32", "32 bit signed integer type.",
            7);
    primitiveTypeKind->AddLiteral("PTK_UInt32", "32 bit unsigned integer type.",
            8);
    primitiveTypeKind->AddLiteral("PTK_Int64", "64 bit signed integer type.",
            9);
    primitiveTypeKind->AddLiteral("PTK_UInt64", "64 bit unsigned integer type.",
            10);
    primitiveTypeKind->AddLiteral("PTK_Float32",
            "32 bit single-precision floating-point type.", 11);
    primitiveTypeKind->AddLiteral("PTK_Float64",
            "64 bit double-precision floating-point type.", 12);
    primitiveTypeKind->AddLiteral("PTK_Duration", "Duration in nanoseconds.",
            13);
    primitiveTypeKind->AddLiteral("PTK_DateTime",
            "Absolute time in nanoseconds.", 14);
    primitiveTypeKind->AddLiteral("PTK_String8", "8 bit character string.", 15);

    AddIntegerType("EventId", //Name
            "Identifier of global event of scheduler or event manager service.", //description
            ::Smp::Uuids::Uuid_EventId, //UUID
            std::numeric_limits<::Smp::Int64>::min(), //minimum
            std::numeric_limits<::Smp::Int64>::max(), //maximum
            "", //unit
            ::Smp::PrimitiveTypeKind::PTK_Int64);

    AddIntegerType("LogMessageKind", //Name
            "This type is used as identifier of a log message kind.", //description
            ::Smp::Uuids::Uuid_LogMessageKind, //UUID
            0, //minimum
            2147483647, //maximum
            "", //unit
            ::Smp::PrimitiveTypeKind::PTK_Int32);

    auto *timeKind = AddEnumerationType("TimeKind", // name
            "Enumeration of supported time kinds.", // description
            ::Smp::Uuids::Uuid_TimeKind, // UUID
            sizeof(::Smp::Services::TimeKind));

    // Register the Literals of the Enumeration
    timeKind->AddLiteral("TK_SimulationTime", "Simulation time.", 0);
    timeKind->AddLiteral("TK_MissionTime", "Mission time.", 1);
    timeKind->AddLiteral("TK_EpochTime", "Epoch time.", 2);
    timeKind->AddLiteral("TK_ZuluTime", "Zulu time.", 3);

    auto *viewKind =
            AddEnumerationType(
                    "ViewKind", // name
                    R"(This enumeration defines possible options for the View attribute, which can be used to control if and how an element is made visible when published to the simulation infrastructure.
The simulation infrastructure must at least support the "None" and the "All" roles (i.e. hidden or always visible).
The simulation infrastructure may support the selection of different user roles, in which case the "Debug" and the "Expert" role must also be supported as described.)", // description
                    ::Smp::Uuids::Uuid_ViewKind, // UUID
                    sizeof(::Smp::ViewKind));

    // Register the Literals of the Enumeration
    viewKind->AddLiteral("VK_None",
            "The element is not made visible to the user (this is the default).",
            0);
    viewKind->AddLiteral("VK_Debug",
            R"(The element is made visible for debugging purposes.
The element is not visible to end users. If the simulation infrastructure supports the selection of different user roles, then the element shall be visible to "Debug" users only.)",
            1);
    viewKind->AddLiteral("VK_Expert",
            R"(The element is made visible for expert users.
The element is not visible to end users. If the simulation infrastructure supports the selection of different user roles, then the element shall be visible to "Debug" and "Expert" users.)",
            2);
    viewKind->AddLiteral("VK_All", "The element is made visible to all users.",
            3);

    auto *parameterDirectionKind =
            AddEnumerationType(
                    "ParameterDirectionKind", // name
                    "The Parameter Direction Kind enumeration defines the possible parameter directions.", // description
                    ::Smp::Uuids::Uuid_ParameterDirectionKind, // UUID
                    sizeof(::Smp::Publication::ParameterDirectionKind));

    // Register the Literals of the Enumeration
    parameterDirectionKind->AddLiteral("PDK_In",
            "The parameter is read-only to the operation, i.e. its value must be specified on call, and cannot be changed inside the operation.",
            0);
    parameterDirectionKind->AddLiteral("PDK_Out",
            "The parameter is write-only to the operation, i.e. its value is unspecified on call, and must be set by the operation.",
            1);
    parameterDirectionKind->AddLiteral("PDK_InOut",
            "The parameter must be specified on call, and may be changed by the operation.",
            2);
    parameterDirectionKind->AddLiteral("PDK_Return",
            "The parameter represents the operation's return value.", 3);

    auto *componentStateKind =
            AddEnumerationType(
                    "ComponentStateKind", // name
                    "This is an enumeration of the available states of a component. Each component is always in one of these four component states.", // description
                    ::Smp::Uuids::Uuid_ComponentStateKind, // UUID
                    sizeof(::Smp::ComponentStateKind));

    // Register the Literals of the Enumeration
    componentStateKind->AddLiteral("CSK_Created",
            "The Created state is the initial state of a component. Component creation is done by an external mechanism, e.g. by factories."
                    "This state is entered automatically after the component has been created."
                    "This state is left via the Publish() state transition.",
            0);
    componentStateKind->AddLiteral("CSK_Publishing",
            "In Publishing state, the component is allowed to publish features. This includes publication of fields, operations and properties. In addition, the component is allowed to create other components."
                    "This state is entered via the Publish() state transition."
                    "This state is left via the Configure() state transition.",
            1);
    componentStateKind->AddLiteral("CSK_Configured",
            "In Configured state, the component has been fully configured. This configuration may be done by external components, or internally by the component itself, e.g. by reading data from an external source."
                    "This state is entered via the Configure() state transition."
                    "This state is left via the Connect() state transition.",
            2);
    componentStateKind->AddLiteral("CSK_Connected",
            "In Connected state, the component is connected to the simulator. In this state, neither publication nor creation of other components is allowed anymore. Configuration performed via loading of SMDL configuration file and/or calling of initialisation entry point are performed in this state."
                    "This state is entered via the Connect() state transition."
                    "This state is left via the Disconnect() state transition or on simulation termination.",
            3);
    componentStateKind->AddLiteral("CSK_Disconnected",
            "In Disconnected state, the component is disconnected from the simulator, and all references to it are deleted, so that it can be deleted."
                    "This state is entered via the Disconnect() state transition."
                    "This is the final state of a component, and only left on deletion.",
            4);

    auto *accessKind =
            AddEnumerationType(
                    "AccessKind", // name
                    "The Access Kind of a property defines whether it has getter and setter.", // description
                    ::Smp::Uuids::Uuid_AccessKind, // UUID
                    sizeof(::Smp::AccessKind));

    // Register the Literals of the Enumeration
    accessKind->AddLiteral("AK_ReadWrite",
            "Read/Write access, i.e. getter and setter.", 0);
    accessKind->AddLiteral("AK_ReadOnly",
            "Read only access, i.e. only getter method.", 1);
    accessKind->AddLiteral("AK_WriteOnly",
            "Write only access, i.e. only setter method.", 2);

    auto *simulatorStateKind =
            AddEnumerationType(
                    "SimulatorStateKind", // name
                    "This is an enumeration of the available states of the simulator. The Setup phase is split into three different states, the Execution phase has five different states, and the Termination phase has two states.", // description
                    ::Smp::Uuids::Uuid_SimulatorStateKind, // UUID
                    sizeof(::Smp::SimulatorStateKind));

    // Register the Literals of the Enumeration
    simulatorStateKind->AddLiteral("SSK_Building",
            "In Building state, the component hierarchy is created. This is done by an external component, not by the simulator."
                    "This state is entered automatically after the simulation environment has performed its initialisation."
                    "In this state, Publish() and Configure() can be called any time to call the corresponding Publish() and Configure() operations of each component."
                    "This state is left with the Connect() state transition method.",
            0);
    simulatorStateKind->AddLiteral("SSK_Connecting",
            "In Connecting state, the simulation environment traverses the component hierarchy and calls the Connect() method of each component."
                    "This state is entered using the Connect() state transition."
                    "After connecting all components to the simulator, an automatic state transition to the Initialising state is performed.",
            1);
    simulatorStateKind->AddLiteral("SSK_Initialising",
            "In Initialising state, the simulation environment executes all initialisation entry points in the order they have been added to the simulator using the AddInitEntryPoint() method."
                    "This state is either entered automatically after the simulation environment has connected all models to the simulator, or manually from Standby state using the Initialise() state transition."
                    "After calling all initialisation entry points, an automatic state transition to the Standby state is performed.",
            2);
    simulatorStateKind->AddLiteral("SSK_Standby",
            "In Standby state, the simulation environment (namely the Time Keeper Service) does not progress simulation time. Only entry points registered relative to Zulu time are executed."
                    "This state is entered automatically from the Initialising, Storing, and Restoring states, or manually from the Executing state using the Hold() state transition."
                    "This state is left with one of the Run(), Store(), Restore(), Initialise(), Reconnect() or Exit() state transitions.",
            3);
    simulatorStateKind->AddLiteral("SSK_Executing",
            "In Executing state, the simulation environment (namely the Time Keeper Service) does progress simulation time. Entry points registered with any of the available time kinds are executed."
                    "This state is entered using the Run() state transition."
                    "This state is left using the Hold() state transition.", 4);
    simulatorStateKind->AddLiteral("SSK_Storing",
            "In Storing state, the simulation environment first stores the values of all fields published with the State attribute to storage (typically a file). Afterwards, the Store() method of all components (Models and Services) implementing the optional IPersist interface is called, to allow custom storing of additional information. While in this state, fields published with the State attribute must not be modified by the models, to ensure that a consistent set of field values is stored."
                    "This state is entered using the Store() state transition."
                    "After storing the simulator state, an automatic state transition to the Standby state is performed.",
            5);
    simulatorStateKind->AddLiteral("SSK_Restoring",
            "In Restoring state, the simulation environment first restores the values of all fields published with the State attribute from storage. Afterwards, the Restore() method of all components implementing the optional IPersist interface is called, to allow custom restoring of additional information. While in this state, fields published with the State attribute must not be modified by the models, to ensure that a consistent set of field values is restored."
                    "This state is entered using the Restore() state transition."
                    "After restoring the simulator state, an automatic state transition to the Standby state is performed.",
            6);
    simulatorStateKind->AddLiteral("SSK_Reconnecting",
            "In Reconnecting state, the simulation environment makes sure that models that have been added to the simulator after leaving the Building state are properly published, configured and connected."
                    "This state is entered using the Reconnect() state transition."
                    "After connecting all new models, an automatic state transition to the Standby state is performed.",
            7);
    simulatorStateKind->AddLiteral("SSK_Exiting",
            "In Exiting state, the simulation environment is properly terminating a running simulation."
                    "This state is entered using the Exit() state transition. After exiting, the simulator is in an undefined state.",
            8);
    simulatorStateKind->AddLiteral("SSK_Aborting",
            "In this state, the simulation environment performs an abnormal simulation shut-down."
                    "This state is entered using the Abort() state transition. After aborting, the simulator is in an undefined state.",
            9);

}
::Smp::String8 TypeRegistry::GetName() const {
    return "TypeRegistry";
}

::Smp::String8 TypeRegistry::GetDescription() const {
    return "Xsmp Type Registry";
}

::Smp::IObject* TypeRegistry::GetParent() const {
    return _parent;
}

::Smp::Publication::IType* TypeRegistry::GetType(
        ::Smp::PrimitiveTypeKind type) const {

    switch (type) {
    case ::Smp::PrimitiveTypeKind::PTK_Bool:
        return GetType(::Smp::Uuids::Uuid_Bool);
    case ::Smp::PrimitiveTypeKind::PTK_Char8:
        return GetType(::Smp::Uuids::Uuid_Char8);
    case ::Smp::PrimitiveTypeKind::PTK_Float32:
        return GetType(::Smp::Uuids::Uuid_Float32);
    case ::Smp::PrimitiveTypeKind::PTK_Float64:
        return GetType(::Smp::Uuids::Uuid_Float64);
    case ::Smp::PrimitiveTypeKind::PTK_Int8:
        return GetType(::Smp::Uuids::Uuid_Int8);
    case ::Smp::PrimitiveTypeKind::PTK_Int16:
        return GetType(::Smp::Uuids::Uuid_Int16);
    case ::Smp::PrimitiveTypeKind::PTK_Int32:
        return GetType(::Smp::Uuids::Uuid_Int32);
    case ::Smp::PrimitiveTypeKind::PTK_Int64:
        return GetType(::Smp::Uuids::Uuid_Int64);
    case ::Smp::PrimitiveTypeKind::PTK_Duration:
        return GetType(::Smp::Uuids::Uuid_Duration);
    case ::Smp::PrimitiveTypeKind::PTK_DateTime:
        return GetType(::Smp::Uuids::Uuid_DateTime);
    case ::Smp::PrimitiveTypeKind::PTK_UInt8:
        return GetType(::Smp::Uuids::Uuid_UInt8);
    case ::Smp::PrimitiveTypeKind::PTK_UInt16:
        return GetType(::Smp::Uuids::Uuid_UInt16);
    case ::Smp::PrimitiveTypeKind::PTK_UInt32:
        return GetType(::Smp::Uuids::Uuid_UInt32);
    case ::Smp::PrimitiveTypeKind::PTK_UInt64:
        return GetType(::Smp::Uuids::Uuid_UInt64);
    case ::Smp::PrimitiveTypeKind::PTK_String8:
        return GetType(::Smp::Uuids::Uuid_String8);
    default:
        return nullptr;
    }

}

::Smp::Publication::IType* TypeRegistry::GetType(::Smp::Uuid typeUuid) const {
    if (auto it = _types.find(typeUuid); it != _types.end())
        return it->second.get();
    return nullptr;
}

template<typename T, class ...Args>
T* TypeRegistry::AddType(Args &&...args) {
    auto type = std::make_unique<T>(std::forward<Args>(args)...);
    if (auto it = _types.find(type->GetUuid()); it != _types.end())
        ::Xsmp::Exception::throwTypeAlreadyRegistered(this, type->GetName(),
                it->second.get());

    auto result = type.get();
    _types.try_emplace(type->GetUuid(), std::move(type));
    return result;
}

::Smp::Publication::IType* TypeRegistry::AddFloatType(::Smp::String8 name,
        ::Smp::String8 description, ::Smp::Uuid typeUuid,
        ::Smp::Float64 minimum, ::Smp::Float64 maximum,
        ::Smp::Bool minInclusive, ::Smp::Bool maxInclusive, ::Smp::String8 unit,
        ::Smp::PrimitiveTypeKind type) {

    switch (type) {
    case ::Smp::PrimitiveTypeKind::PTK_Float32:
    case ::Smp::PrimitiveTypeKind::PTK_Float64:
        return AddType<FloatType>(name, description, this, typeUuid, minimum,
                maximum, minInclusive, maxInclusive, unit, type);
    default:
        ::Xsmp::Exception::throwInvalidPrimitiveType(this, name, type);
    }

}

::Smp::Publication::IType* TypeRegistry::AddIntegerType(::Smp::String8 name,
        ::Smp::String8 description, ::Smp::Uuid typeUuid, ::Smp::Int64 minimum,
        ::Smp::Int64 maximum, ::Smp::String8 unit,
        ::Smp::PrimitiveTypeKind type) {

    switch (type) {
    case ::Smp::PrimitiveTypeKind::PTK_Int8:
    case ::Smp::PrimitiveTypeKind::PTK_Int16:
    case ::Smp::PrimitiveTypeKind::PTK_Int32:
    case ::Smp::PrimitiveTypeKind::PTK_Int64:
    case ::Smp::PrimitiveTypeKind::PTK_UInt8:
    case ::Smp::PrimitiveTypeKind::PTK_UInt16:
    case ::Smp::PrimitiveTypeKind::PTK_UInt32:
    case ::Smp::PrimitiveTypeKind::PTK_UInt64:
        return AddType<IntegerType>(name, description, this, typeUuid, minimum,
                maximum, unit, type);
    default:
        ::Xsmp::Exception::throwInvalidPrimitiveType(this, name, type);
    }

}

::Smp::Publication::IEnumerationType* TypeRegistry::AddEnumerationType(
        ::Smp::String8 name, ::Smp::String8 description, ::Smp::Uuid typeUuid,
        ::Smp::Int16 memorySize) {
    return AddType<EnumerationType>(name, description, this, typeUuid,
            memorySize);
}

::Smp::Publication::IArrayType* TypeRegistry::AddArrayType(::Smp::String8 name,
        ::Smp::String8 description, ::Smp::Uuid typeUuid,
        ::Smp::Uuid itemTypeUuid, ::Smp::Int64 itemSize,
        ::Smp::Int64 arrayCount, ::Smp::Bool simpleArray) {
    return AddType<ArrayType>(name, description, this, typeUuid, itemTypeUuid,
            itemSize, arrayCount, simpleArray);
}

::Smp::Publication::IType* TypeRegistry::AddStringType(::Smp::String8 name,
        ::Smp::String8 description, ::Smp::Uuid typeUuid, ::Smp::Int64 length) {
    return AddType<StringType>(name, description, this, typeUuid, length);
}

::Smp::Publication::IStructureType* TypeRegistry::AddStructureType(
        ::Smp::String8 name, ::Smp::String8 description, ::Smp::Uuid typeUuid) {
    return AddType<StructureType>(name, description, this, typeUuid);
}

::Smp::Publication::IClassType* TypeRegistry::AddClassType(::Smp::String8 name,
        ::Smp::String8 description, ::Smp::Uuid typeUuid,
        ::Smp::Uuid baseClassUuid) {

    return AddType<ClassType>(name, description, this, typeUuid, baseClassUuid);
}

} // namespace Xsmp::Publication
