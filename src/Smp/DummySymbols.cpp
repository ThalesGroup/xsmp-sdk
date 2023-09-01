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

#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/EventId.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ILinkRegistry.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/IResolver.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Smp/Services/LogMessageKind.h>
#include <Smp/Uuid.h>

namespace Smp {
namespace Services {

// Dummy definition of IEventManager constexpr symbols
constexpr ::Smp::Char8 IEventManager::SMP_EventManager[];
constexpr EventId IEventManager::SMP_LeaveConnectingId;
constexpr ::Smp::Char8 IEventManager::SMP_LeaveConnecting[];
constexpr EventId IEventManager::SMP_EnterInitialisingId;
constexpr ::Smp::Char8 IEventManager::SMP_EnterInitialising[];
constexpr EventId IEventManager::SMP_LeaveInitialisingId;
constexpr ::Smp::Char8 IEventManager::SMP_LeaveInitialising[];
constexpr EventId IEventManager::SMP_EnterStandbyId;
constexpr ::Smp::Char8 IEventManager::SMP_EnterStandby[];
constexpr EventId IEventManager::SMP_LeaveStandbyId;
constexpr ::Smp::Char8 IEventManager::SMP_LeaveStandby[];
constexpr EventId IEventManager::SMP_EnterExecutingId;
constexpr ::Smp::Char8 IEventManager::SMP_EnterExecuting[];
constexpr EventId IEventManager::SMP_LeaveExecutingId;
constexpr ::Smp::Char8 IEventManager::SMP_LeaveExecuting[];
constexpr EventId IEventManager::SMP_EnterStoringId;
constexpr ::Smp::Char8 IEventManager::SMP_EnterStoring[];
constexpr EventId IEventManager::SMP_LeaveStoringId;
constexpr ::Smp::Char8 IEventManager::SMP_LeaveStoring[];
constexpr EventId IEventManager::SMP_EnterRestoringId;
constexpr ::Smp::Char8 IEventManager::SMP_EnterRestoring[];
constexpr EventId IEventManager::SMP_LeaveRestoringId;
constexpr ::Smp::Char8 IEventManager::SMP_LeaveRestoring[];
constexpr EventId IEventManager::SMP_EnterExitingId;
constexpr ::Smp::Char8 IEventManager::SMP_EnterExiting[];
constexpr EventId IEventManager::SMP_EnterAbortingId;
constexpr ::Smp::Char8 IEventManager::SMP_EnterAborting[];
constexpr EventId IEventManager::SMP_EpochTimeChangedId;
constexpr ::Smp::Char8 IEventManager::SMP_EpochTimeChanged[];
constexpr EventId IEventManager::SMP_MissionTimeChangedId;
constexpr ::Smp::Char8 IEventManager::SMP_MissionTimeChanged[];
constexpr EventId IEventManager::SMP_EnterReconnectingId;
constexpr ::Smp::Char8 IEventManager::SMP_EnterReconnecting[];
constexpr EventId IEventManager::SMP_LeaveReconnectingId;
constexpr ::Smp::Char8 IEventManager::SMP_LeaveReconnecting[];
constexpr EventId IEventManager::SMP_PreSimTimeChangeId;
constexpr ::Smp::Char8 IEventManager::SMP_PreSimTimeChange[];
constexpr EventId IEventManager::SMP_PostSimTimeChangeId;
constexpr ::Smp::Char8 IEventManager::SMP_PostSimTimeChange[];

// Dummy definition of ILinkRegistry constexpr symbols
constexpr ::Smp::Char8 ILinkRegistry::SMP_LinkRegistry[];

// Dummy definition of ILogger constexpr symbols
constexpr ::Smp::Char8 ILogger::LMK_InformationName[];
constexpr ::Smp::Char8 ILogger::LMK_DebugName[];
constexpr ::Smp::Char8 ILogger::LMK_ErrorName[];
constexpr ::Smp::Char8 ILogger::LMK_WarningName[];
constexpr ::Smp::Char8 ILogger::LMK_EventName[];
constexpr ::Smp::Char8 ILogger::SMP_Logger[];
constexpr LogMessageKind ILogger::LMK_Information;
constexpr LogMessageKind ILogger::LMK_Event;
constexpr LogMessageKind ILogger::LMK_Warning;
constexpr LogMessageKind ILogger::LMK_Error;
constexpr LogMessageKind ILogger::LMK_Debug;

// Dummy definition of IResolver constexpr symbols
constexpr ::Smp::Char8 IResolver::SMP_Resolver[];

// Dummy definition of IScheduler constexpr symbols
constexpr ::Smp::Char8 IScheduler::SMP_Scheduler[];

// Dummy definition of ITimeKeeper constexpr symbols
constexpr ::Smp::Char8 ITimeKeeper::SMP_TimeKeeper[];

} // namespace Services

// Dummy definition of ISimulator constexpr symbols
constexpr ::Smp::Char8 ISimulator::SMP_SimulatorServices[];
constexpr ::Smp::Char8 ISimulator::SMP_SimulatorModels[];

// Dummy definition of Uuids constexpr symbols
constexpr ::Smp::Uuid Uuids::Uuid_Uuid;
constexpr ::Smp::Uuid Uuids::Uuid_Void;
constexpr ::Smp::Uuid Uuids::Uuid_Char8;
constexpr ::Smp::Uuid Uuids::Uuid_Bool;
constexpr ::Smp::Uuid Uuids::Uuid_Int8;
constexpr ::Smp::Uuid Uuids::Uuid_UInt8;
constexpr ::Smp::Uuid Uuids::Uuid_Int16;
constexpr ::Smp::Uuid Uuids::Uuid_UInt16;
constexpr ::Smp::Uuid Uuids::Uuid_Int32;
constexpr ::Smp::Uuid Uuids::Uuid_UInt32;
constexpr ::Smp::Uuid Uuids::Uuid_Int64;
constexpr ::Smp::Uuid Uuids::Uuid_UInt64;
constexpr ::Smp::Uuid Uuids::Uuid_Float32;
constexpr ::Smp::Uuid Uuids::Uuid_Float64;
constexpr ::Smp::Uuid Uuids::Uuid_Duration;
constexpr ::Smp::Uuid Uuids::Uuid_DateTime;
constexpr ::Smp::Uuid Uuids::Uuid_String8;
constexpr ::Smp::Uuid Uuids::Uuid_PrimitiveTypeKind;
constexpr ::Smp::Uuid Uuids::Uuid_EventId;
constexpr ::Smp::Uuid Uuids::Uuid_LogMessageKind;
constexpr ::Smp::Uuid Uuids::Uuid_TimeKind;
constexpr ::Smp::Uuid Uuids::Uuid_ViewKind;
constexpr ::Smp::Uuid Uuids::Uuid_ParameterDirectionKind;
constexpr ::Smp::Uuid Uuids::Uuid_ComponentStateKind;
constexpr ::Smp::Uuid Uuids::Uuid_AccessKind;
constexpr ::Smp::Uuid Uuids::Uuid_SimulatorStateKind;
} // namespace Smp
