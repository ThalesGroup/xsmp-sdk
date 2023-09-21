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


//#include <python/ecss_smp.h>
#include <python/Smp/AccessKindBinding.h>
#include <python/Smp/ComponentStateKindBinding.h>
#include <python/Smp/IAggregateBinding.h>
#include <python/Smp/IArrayFieldBinding.h>
#include <python/Smp/IComponentBinding.h>
#include <python/Smp/ICompositeBinding.h>
#include <python/Smp/IContainerBinding.h>
#include <python/Smp/IDataflowFieldBinding.h>
#include <python/Smp/IDynamicInvocationBinding.h>
#include <python/Smp/IEntryPointBinding.h>
#include <python/Smp/IEntryPointPublisherBinding.h>
#include <python/Smp/IEventConsumerBinding.h>
#include <python/Smp/IEventProviderBinding.h>
#include <python/Smp/IEventSinkBinding.h>
#include <python/Smp/IEventSourceBinding.h>
#include <python/Smp/IFailureBinding.h>
#include <python/Smp/IFallibleModelBinding.h>
#include <python/Smp/IFieldBinding.h>
#include <python/Smp/IForcibleFieldBinding.h>
#include <python/Smp/ILinkingComponentBinding.h>
#include <python/Smp/IModelBinding.h>
#include <python/Smp/IObjectBinding.h>
#include <python/Smp/IOperationBinding.h>
#include <python/Smp/IPersistBinding.h>
#include <python/Smp/IPropertyBinding.h>
#include <python/Smp/IReferenceBinding.h>
#include <python/Smp/IServiceBinding.h>
#include <python/Smp/ISimpleArrayFieldBinding.h>
#include <python/Smp/ISimpleFieldBinding.h>
#include <python/Smp/ISimulatorBinding.h>
#include <python/Smp/IStructureFieldBinding.h>
#include <python/Smp/Services/IEventManagerBinding.h>
#include <python/Smp/Services/ILinkRegistryBinding.h>
#include <python/Smp/Services/ILoggerBinding.h>
#include <python/Smp/Services/IResolverBinding.h>
#include <python/Smp/Services/ISchedulerBinding.h>
#include <python/Smp/Services/ITimeKeeperBinding.h>
#include <python/Smp/Services/TimeKindBinding.h>
#include <python/Smp/SimulatorStateKindBinding.h>
#include <python/Smp/UuidBinding.h>
#include <python/Smp/ViewKindBinding.h>

#include <Smp/Exception.h>
#include <Smp/CannotDelete.h>
#include <Smp/CannotRemove.h>
#include <Smp/CannotRestore.h>
#include <Smp/CannotStore.h>
#include <Smp/ContainerFull.h>
#include <Smp/DuplicateName.h>
#include <Smp/DuplicateUuid.h>
#include <Smp/EventSinkAlreadySubscribed.h>
#include <Smp/EventSinkNotSubscribed.h>
#include <Smp/FieldAlreadyConnected.h>
#include <Smp/InvalidAnyType.h>
#include <Smp/InvalidArrayIndex.h>
#include <Smp/InvalidArraySize.h>
#include <Smp/InvalidArrayValue.h>
#include <Smp/InvalidComponentState.h>
#include <Smp/InvalidEventSink.h>
#include <Smp/InvalidFieldName.h>
#include <Smp/InvalidFieldType.h>
#include <Smp/InvalidFieldValue.h>
#include <Smp/InvalidLibrary.h>
#include <Smp/InvalidObjectName.h>
#include <Smp/InvalidObjectType.h>
#include <Smp/InvalidOperationName.h>
#include <Smp/InvalidParameterCount.h>
#include <Smp/InvalidParameterIndex.h>
#include <Smp/InvalidParameterType.h>
#include <Smp/InvalidParameterValue.h>
#include <Smp/InvalidReturnValue.h>
#include <Smp/InvalidSimulatorState.h>
#include <Smp/InvalidTarget.h>
#include <Smp/LibraryNotFound.h>
#include <Smp/NotContained.h>
#include <Smp/NotReferenced.h>
#include <Smp/Services/EntryPointAlreadySubscribed.h>
#include <Smp/Services/EntryPointNotSubscribed.h>
#include <Smp/Services/InvalidCycleTime.h>
#include <Smp/Services/InvalidEventId.h>
#include <Smp/Services/InvalidEventName.h>
#include <Smp/Services/InvalidEventTime.h>
#include <Smp/Services/InvalidSimulationTime.h>
#include <Smp/Publication/DuplicateLiteral.h>
#include <Smp/Publication/InvalidPrimitiveType.h>
#include <Smp/Publication/TypeAlreadyRegistered.h>
#include <Smp/Publication/TypeNotRegistered.h>
#include <Smp/ReferenceFull.h>
#include <Smp/VoidOperation.h>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

//namespace py = pybind11;

PYBIND11_MODULE(ecss_smp, ecss_smp) {
    ecss_smp.doc() = R"(Specifies the SMP Component Model as SMDL Catalogue.)";

#ifdef VERSION_INFO
    ecss_smp.attr("__version__") = MACRO_STRINGIFY(VERSION_INFO);
#else
    ecss_smp.attr("__version__") = "dev";
#endif

    auto smp = ecss_smp.def_submodule("Smp",
            "SMP standard types and interfaces.");

    const auto &Exception = py::register_exception<::Smp::Exception>(smp,
            "Exception", PyExc_RuntimeError);
    Exception.doc() =
            R"(This is the base class for all SMP exceptions.
This exception is the base class for all other SMP exceptions. It provides Name, Description and Message.)";

#define SMP_EXCEPTION(x) py::register_exception<::Smp::x> (smp, #x, Exception)

    SMP_EXCEPTION(CannotDelete).doc() =
            "This exception is thrown when trying to delete a component from a container when the number of "
                    "contained components is lower than or equal to the Lower limit.";

    SMP_EXCEPTION(CannotRemove).doc() =
            "This exception is thrown when trying to remove a component from a reference when the number of "
                    "referenced components is lower than or equal to the Lower limit.";

    SMP_EXCEPTION(CannotRestore).doc() =
            R"(This exception is raised when the content of the storage reader passed to the Restore() method contains invalid data.
@remarks This typically happens when a Store() has been created from a different configuration of objects.)";

    SMP_EXCEPTION(CannotStore).doc() =
            R"(This exception is raised when the component cannot store its data to the storage writer given to the Store() method.
@remarks This may e.g. be if there is no disk space left.)";

    SMP_EXCEPTION(ContainerFull).doc() =
            "This exception is raised when trying to add a component to a container that is full, "
                    "i.e. where the Count has reached the Upper limit.";

    SMP_EXCEPTION(DuplicateName).doc() =
            "This exception is raised when trying to add an object to a collection of objects, "
                    "which have to have unique names, but another object with the same name does exist already in this collection. "
                    "This would lead to duplicate names.";

    SMP_EXCEPTION(DuplicateUuid).doc() =
            "This exception is raised when trying to register a factory under a Uuid that has already been used to register "
                    "another (or the same) factory. This would lead to duplicate implementation Uuids.";

    SMP_EXCEPTION(EventSinkAlreadySubscribed).doc() =
            "This exception is raised when trying to subscribe an event sink to an event source that is already subscribed.";

    SMP_EXCEPTION(EventSinkNotSubscribed).doc() =
            "This exception is raised when trying to unsubscribe an event sink from an event source that is not subscribed to it.";

    SMP_EXCEPTION(FieldAlreadyConnected).doc() =
            "This exception is raised when trying to connect a target field to a data flow field that is already connected.";

    SMP_EXCEPTION(InvalidAnyType).doc() =
            R"(This exception is raised when trying to use an AnySimple argument of wrong type.
@remarks This can happen when assigning a value to an AnySimple  instance, but as well when e.g. registering an event sink with an event source of another event argument type.)";

    SMP_EXCEPTION(InvalidArrayIndex).doc() =
            "This exception is raised when an invalid index is specified.";

    SMP_EXCEPTION(InvalidArraySize).doc() =
            "This exception is raised when an invalid array size is specified.";

    SMP_EXCEPTION(InvalidArrayValue).doc() =
            "This exception is raised when trying to assign an illegal value to an array field.";

    SMP_EXCEPTION(InvalidComponentState).doc() =
            "This exception is raised by a component when one of the state transition commands is called in an invalid state.";

    SMP_EXCEPTION(InvalidEventSink).doc() =
            "This exception is raised when trying to subscribe an event sink to an event source that has a different event type.";

    SMP_EXCEPTION(InvalidFieldName).doc() =
            "This exception is raised when an invalid field name is specified.";

    SMP_EXCEPTION(InvalidFieldType).doc() =
            R"(Invalid field type.
This exception is raised when trying to publish a field with invalid type.
@remarks This can happen, for example, when trying to publish a field of the variable-length simple type String8.)";

    SMP_EXCEPTION(InvalidFieldValue).doc() =
            "This exception is raised when trying to assign an illegal value to a field.";

    SMP_EXCEPTION(InvalidLibrary).doc() =
            "This exception is raised when trying to load a library that does not contain an Initialise() function.";

    SMP_EXCEPTION(InvalidObjectName).doc() =
            R"(This exception is raised when trying to set an object's name to an invalid name. Names
     - must not be empty,
     - must start with a letter, and
     - must only contain letters, digits, the underscore ("_") and  brackets ("[" and "]").)";

    SMP_EXCEPTION(InvalidObjectType).doc() =
            R"(This exception is raised when trying to pass an object of wrong type.
@remarks This can happen when adding a component to a container or reference which is semantically typed by a specific type implementing IComponent.)";

    SMP_EXCEPTION(InvalidOperationName).doc() =
            "This exception is raised by the Invoke() method when trying to invoke a method that does not exist, or that does not support dynamic invocation.";

    SMP_EXCEPTION(InvalidParameterCount).doc() =
            "This exception is raised by the Invoke() method when trying to invoke a method with a wrong number of parameters.";

    SMP_EXCEPTION(InvalidParameterIndex).doc() =
            "This exception is raised when using an invalid parameter index to set (SetParameterValue()) or get (GetParameterValue())"
                    " a parameter value of an operation in a request.";

    SMP_EXCEPTION(InvalidParameterType).doc() =
            R"(This exception is raised by the Invoke() method when trying to invoke a method passing a parameter of wrong type.
@remarks The index of the parameter of wrong type can be extracted from the request using the method GetParameterIndex().)";

    SMP_EXCEPTION(InvalidParameterValue).doc() =
            "This exception is raised when trying to assign an illegal value to a parameter of an operation in a request using SetParameterValue().";

    SMP_EXCEPTION(InvalidReturnValue).doc() =
            "This exception is raised when trying to assign an invalid return value of an operation in a request using SetReturnValue().";

    SMP_EXCEPTION(InvalidSimulatorState).doc() =
            "This exception is raised by the simulator when one of the operations is called in an invalid state.";

    SMP_EXCEPTION(InvalidTarget).doc() =
            "This exception is raised when trying to connect two data flow fields of incompatible types.";

    SMP_EXCEPTION(LibraryNotFound).doc() =
            "This exception is raised when trying to load a library that does not exist.";

    SMP_EXCEPTION(NotContained).doc() =
            "This exception is thrown when trying to delete a component from a container which is not contained.";

    SMP_EXCEPTION(NotReferenced).doc() =
            "This exception is thrown when trying to remove a component from a reference which was not referenced before.";

    SMP_EXCEPTION(ReferenceFull).doc() =
            "This exception is raised when trying to add a component to a reference that is full, i.e. where the Count has reached the Upper limit.";

    SMP_EXCEPTION(VoidOperation).doc() =
            "This exception is raised when trying to read (GetReturnValue()) or write (SetReturnValue()) the return value of a void operation.";

#undef SMP_EXCEPTION

    RegisterUuid(smp);
    RegisterAccessKind(smp);
    RegisterComponentStateKind(smp);
    RegisterSimulatorStateKind(smp);
    RegisterViewKind(smp);

    RegisterIObject(smp);

    RegisterIOperation(smp);
    RegisterIEntryPointPublisher(smp);

    RegisterIEntryPoint(smp);
    RegisterIProperty(smp);
    RegisterIEventSink(smp);
    RegisterIEventSource(smp);

    RegisterIPersist(smp);
    RegisterIFailure(smp);
    RegisterIField(smp);
    RegisterIDataflowField(smp);
    RegisterISimpleField(smp);
    RegisterIForcibleField(smp);
    RegisterIArrayField(smp);
    RegisterISimpleArrayField(smp);
    RegisterIStructureField(smp);

    RegisterIContainer(smp);
    RegisterIReference(smp);

    RegisterIComponent(smp);
    RegisterIAggregate(smp);
    RegisterIDynamicInvocation(smp);
    RegisterIEventConsumer(smp);
    RegisterIEventProvider(smp);
    RegisterILinkingComponent(smp);

    RegisterIModel(smp);
    RegisterIFallibleModel(smp);

    RegisterIService(smp);

    RegisterIComposite(smp);

    auto publication = smp.def_submodule("Publication",
            "Namespace for publication");

    py::register_exception<::Smp::Publication::DuplicateLiteral>(publication,
            "DuplicateLiteral", Exception).doc() =
            "This exception is raised when trying to add a literal to an enumeration using a value that has been used for another literal before.";

    py::register_exception<::Smp::Publication::InvalidPrimitiveType>(
            publication, "InvalidPrimitiveType", Exception).doc() =
            "This exception is raised when trying to use an invalid primitive type kind as parameter for a user-defined float or integer type.";

    py::register_exception<::Smp::Publication::TypeAlreadyRegistered>(
            publication, "TypeAlreadyRegistered", Exception).doc() =
            "This exception is raised when trying to register a type with a Uuid that has already been registered.";

    py::register_exception<::Smp::Publication::TypeNotRegistered>(publication,
            "TypeNotRegistered", Exception).doc() =
            "This exception is raised when trying to publish a feature with a type Uuid that has not been registered.";

    auto services = smp.def_submodule("Services",
            "Namespace for simulation services");

    py::register_exception<::Smp::Services::EntryPointAlreadySubscribed>(
            services, "EntryPointAlreadySubscribed", Exception).doc() =
            "This exception is raised when trying to subscribe an entry point to an event that is already subscribed.";

    py::register_exception<::Smp::Services::EntryPointNotSubscribed>(services,
            "EntryPointNotSubscribed", Exception).doc() =
            "This exception is raised when trying to unsubscribe an entry point from an event that is not subscribed to it.";

    py::register_exception<::Smp::Services::InvalidCycleTime>(services,
            "InvalidCycleTime", Exception).doc() =
            "This exception is thrown by one of the AddEvent() methods of the scheduler when the event is a cyclic event "
                    "(i.e. repeat is not 0), but the cycle time specified is not a positive duration.";

    py::register_exception<::Smp::Services::InvalidEventId>(services,
            "InvalidEventId", Exception).doc() =
            "This exception is raised when an invalid event id is provided, e.g. when calling Subscribe(), Unsubscribe() "
                    "or Emit() of the Event Manager (using an invalid global event id), or when calling SetEventSimulationTime(), "
                    "SetEventMissionTime(), SetEventEpochTime(), SetEventZuluTime(), SetEventCycleTime(), SetEventCount() or "
                    "RemoveEvent() of the Scheduler (using an invalid scheduler event id).";

    py::register_exception<::Smp::Services::InvalidEventName>(services,
            "InvalidEventName", Exception).doc() =
            "This exception is thrown by the QueryEventId() method of the event manager when an empty event name has been provided.";

    py::register_exception<::Smp::Services::InvalidEventTime>(services,
            "InvalidEventTime", Exception).doc() =
            "This exception is thrown by one of the AddEvent() methods of the scheduler when the time specified for the first execution of the event is in the past.";

    py::register_exception<::Smp::Services::InvalidSimulationTime>(services,
            "InvalidSimulationTime", Exception).doc() =
            "This exception is thrown by SetSimulationTime if the new simulation time is not between the current simulation "
                    "time and the simulation time of the next event on the scheduler.";

    RegisterTimeKind(services);
    RegisterIEventManager(services);
    RegisterILinkRegistry(services);
    RegisterILogger(services);
    RegisterIResolver(services);
    RegisterIScheduler(services);
    RegisterITimeKeeper(services);

    RegisterISimulator(smp);

}

