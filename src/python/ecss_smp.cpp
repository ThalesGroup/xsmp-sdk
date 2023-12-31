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

#include <python/ecss_smp.h>
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
#include <Smp/IAggregate.h>
#include <Smp/IArrayField.h>
#include <Smp/IContainer.h>
#include <Smp/IDataflowField.h>
#include <Smp/IDynamicInvocation.h>
#include <Smp/IEntryPoint.h>
#include <Smp/IEntryPointPublisher.h>
#include <Smp/IEventConsumer.h>
#include <Smp/IEventProvider.h>
#include <Smp/IEventSink.h>
#include <Smp/IEventSource.h>
#include <Smp/IFailure.h>
#include <Smp/IFallibleModel.h>
#include <Smp/IForcibleField.h>
#include <Smp/ILinkingComponent.h>
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
#include <Smp/IOperation.h>
#include <Smp/IProperty.h>
#include <Smp/IReference.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimulator.h>
#include <Smp/IStructureField.h>
#include <Smp/LibraryNotFound.h>
#include <Smp/NotContained.h>
#include <Smp/NotReferenced.h>
#include <Smp/Publication/DuplicateLiteral.h>
#include <Smp/Publication/InvalidPrimitiveType.h>
#include <Smp/Publication/TypeAlreadyRegistered.h>
#include <Smp/Publication/TypeNotRegistered.h>
#include <Smp/ReferenceFull.h>
#include <Smp/Services/EntryPointAlreadySubscribed.h>
#include <Smp/Services/EntryPointNotSubscribed.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ILinkRegistry.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/InvalidCycleTime.h>
#include <Smp/Services/InvalidEventId.h>
#include <Smp/Services/InvalidEventName.h>
#include <Smp/Services/InvalidEventTime.h>
#include <Smp/Services/InvalidSimulationTime.h>
#include <Smp/Services/IResolver.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Smp/VoidOperation.h>
#include <Xsmp/Helper.h>
#include <algorithm>
#include <memory>
#include <string>
#include <vector>

#define STRINGIFY(x) #x
#define MACRO_STRINGIFY(x) STRINGIFY(x)

namespace PYBIND11_NAMESPACE {

struct TypeHierarchy {

    template<typename T>
    static TypeHierarchy of(std::vector<TypeHierarchy> _derived = { }) {
        return TypeHierarchy { typeid(T), [](void *src) -> void* {
            return dynamic_cast<T*>(static_cast<::Smp::IObject*>(src));
        },
        [](const ::Smp::IObject *src) -> bool {
            return dynamic_cast<const T*>(src);
        },
        std::move(_derived) };
    }
    const std::type_info &base;
    void* (*caster)(void*);
    bool (*isInstance)(const ::Smp::IObject*);
    std::vector<TypeHierarchy> childs;
};

static inline const TypeHierarchy IObjectHierarchy =
        TypeHierarchy::template of<::Smp::IObject>(
                {
                        //IObject
                        TypeHierarchy::template of<::Smp::IComponent>(
                                {
                                        //IComponent
                                        TypeHierarchy::template of<
                                                ::Smp::IAggregate>(),

                                        TypeHierarchy::template of<
                                                ::Smp::IDynamicInvocation>(),

                                        TypeHierarchy::template of<
                                                ::Smp::IEventConsumer>(),

                                        TypeHierarchy::template of<
                                                ::Smp::IEventProvider>(),

                                        TypeHierarchy::template of<
                                                ::Smp::ILinkingComponent>(),

                                        TypeHierarchy::template of<::Smp::IModel>(
                                                {
                                                //IModel
                                                        TypeHierarchy::template of<
                                                                ::Smp::IFallibleModel>(),

                                                }),

                                        TypeHierarchy::template of<
                                                ::Smp::IService>(
                                                {
                                                        //IService
                                                        TypeHierarchy::template of<
                                                                ::Smp::Services::IEventManager>(),

                                                        TypeHierarchy::template of<
                                                                ::Smp::Services::ILinkRegistry>(),

                                                        TypeHierarchy::template of<
                                                                ::Smp::Services::ILogger>(),

                                                        TypeHierarchy::template of<
                                                                ::Smp::Services::IResolver>(),

                                                        TypeHierarchy::template of<
                                                                ::Smp::Services::IScheduler>(),

                                                        TypeHierarchy::template of<
                                                                ::Smp::Services::ITimeKeeper>(),

                                                }),

                                }),

                        TypeHierarchy::template of<::Smp::IComposite>( {
                        //IComposite
                                TypeHierarchy::template of<::Smp::ISimulator>(),

                        }),

                        TypeHierarchy::template of<::Smp::IContainer>(),

                        TypeHierarchy::template of<::Smp::IEntryPoint>(),

                        TypeHierarchy::template of<::Smp::IEntryPointPublisher>(),

                        TypeHierarchy::template of<::Smp::IEventSink>(),

                        TypeHierarchy::template of<::Smp::IEventSource>(),

                        TypeHierarchy::template of<::Smp::IOperation>(),

                        TypeHierarchy::template of<::Smp::IPersist>(
                                {
                                        // IPersist
                                        TypeHierarchy::template of<
                                                ::Smp::IFailure>(),

                                        TypeHierarchy::template of<::Smp::IField>(
                                                {
                                                        //IField
                                                        TypeHierarchy::template of<
                                                                ::Smp::IArrayField>(),

                                                        TypeHierarchy::template of<
                                                                ::Smp::ISimpleArrayField>(),

                                                        TypeHierarchy::template of<
                                                                ::Smp::ISimpleField>(
                                                                {
                                                                //ISimpleField
                                                                        TypeHierarchy::template of<
                                                                                ::Smp::IForcibleField>(),

                                                                }),

                                                        TypeHierarchy::template of<
                                                                ::Smp::IStructureField>(),
                                                        TypeHierarchy::template of<
                                                                ::Smp::IDataflowField>(),

                                                }),

                                }),

                        TypeHierarchy::template of<::Smp::IProperty>(),

                        TypeHierarchy::template of<::Smp::IReference>(),

                });

class IObjectClass: public detail::generic_type {

public:
    using type = ::Smp::IObject;
    using holder_type = std::unique_ptr<type>;
    explicit IObjectClass(const type *obj) {

        detail::type_record record;
        auto type_name = detail::clean_type_id(typeid(*obj).name());
        record.name = type_name.c_str(); // TODO add random ID ?
        record.type = &typeid(*obj);
        record.type_size = sizeof(*obj);
        record.type_align = alignof(type);
        // Store the dynamic class in the root module ecss_smp
        record.scope = module_::import("ecss_smp");

        record.holder_size = sizeof(holder_type);
        record.init_instance = init_instance;
        record.multiple_inheritance = true;

        record.dealloc = dealloc;
        record.default_holder = true;
        record.module_local = true;
        record.is_final = true;

        processHierarchy(IObjectHierarchy, obj, record);

        detail::generic_type::initialize(record);

    }

    static bool processHierarchy(const TypeHierarchy &hierarchy,
            const type *object, detail::type_record &rec) {
        bool ignore = false;

        for (const auto &elem : hierarchy.childs) {
            ignore |= processHierarchy(elem, object, rec);
        }
        if (!ignore && hierarchy.isInstance(object)) {
            rec.add_base(hierarchy.base, hierarchy.caster);
            ignore = true;
        }
        return ignore;
    }

    /// Performs instance initialization including constructing a holder and registering the known
    /// instance.  Should be called as soon as the `type` value_ptr is set for an instance.  Takes
    /// an optional pointer to an existing holder to use; if not specified and the instance is
    /// `.owned`, a new holder will be constructed to manage the value pointer.
    static void init_instance(detail::instance *inst, const void *holder_ptr) {
        auto v_h = detail::value_and_holder(inst,
                detail::get_type_info(typeid(type)), 0, 0);
        if (!v_h.instance_registered()) {
            register_instance(inst, v_h.value_ptr(), v_h.type);
            v_h.set_instance_registered();
        }
        init_holder(inst, v_h, static_cast<const holder_type*>(holder_ptr), //TODO cast
                v_h.value_ptr<type>());
    }

    /// Deallocates an instance; via holder, if constructed; otherwise via operator delete.
    static void dealloc(detail::value_and_holder &v_h) {
        // We could be deallocating because we are cleaning up after a Python exception.
        // If so, the Python error indicator will be set. We need to clear that before
        // running the destructor, in case the destructor code calls more Python.
        // If we don't, the Python API will exit with an exception, and pybind11 will
        // throw error_already_set from the C++ destructor which is forbidden and triggers
        // std::terminate().
        error_scope scope;
        if (v_h.holder_constructed()) {
            v_h.holder<holder_type>().~holder_type();
            v_h.set_holder_constructed(false);
        }
        else {
            detail::call_operator_delete(v_h.value_ptr<type>(),
                    v_h.type->type_size, v_h.type->type_align);
        }
        v_h.value_ptr() = nullptr;
    }

private:

    static void init_holder_from_existing(const detail::value_and_holder &v_h,
            const holder_type *holder_ptr,
            std::false_type /*is_copy_constructible*/) {
        new (std::addressof(v_h.holder<holder_type>())) holder_type(
                std::move(*const_cast<holder_type*>(holder_ptr)));
    }

    /// Initialize holder object, variant 2: try to construct from existing holder object, if
    /// possible
    static void init_holder(const detail::instance *inst,
            detail::value_and_holder &v_h, const holder_type *holder_ptr,
            const void* /* dummy -- not enable_shared_from_this<T>) */) {
        if (holder_ptr) {
            init_holder_from_existing(v_h, holder_ptr,
                    std::is_copy_constructible<holder_type>());
            v_h.set_holder_constructed();
        }
        else if (detail::always_construct_holder<holder_type>::value
                || inst->owned) {
            new (std::addressof(v_h.holder<holder_type>())) holder_type(
                    v_h.value_ptr<type>());
            v_h.set_holder_constructed();
        }
    }
};
} // namespace PYBIND11_NAMESPACE

const void* IObjectHook(const ::Smp::IObject *src,
        const std::type_info *&type) {

    if (src) {
        type = &typeid(*src);
        if (py::detail::get_local_type_info(*type) == nullptr) {
            py::IObjectClass(src).doc() = "Automatic Python binding for '"
                    + Xsmp::Helper::TypeName(src) + "'.";
        }

        // return a pointer to IObject (instead of dynamic_cast<const void*> )
        return static_cast<const Smp::IObject*>(src);
    }
    type = nullptr;
    return nullptr;

}

py::object convert(const ::Smp::AnySimple &value) {
    switch (value.GetType()) {
    case ::Smp::PrimitiveTypeKind::PTK_String8:
        return py::cast(value.operator ::Smp::String8());
    case ::Smp::PrimitiveTypeKind::PTK_Char8:
        return py::cast(value.operator ::Smp::Char8());
    case ::Smp::PrimitiveTypeKind::PTK_Bool:
        return py::cast(value.operator ::Smp::Bool());
    case ::Smp::PrimitiveTypeKind::PTK_Int8:
        return py::cast(value.operator ::Smp::Int8());
    case ::Smp::PrimitiveTypeKind::PTK_Int16:
        return py::cast(value.operator ::Smp::Int16());
    case ::Smp::PrimitiveTypeKind::PTK_Int32:
        return py::cast(value.operator ::Smp::Int32());
    case ::Smp::PrimitiveTypeKind::PTK_Int64:
        return py::cast(value.operator ::Smp::Int64());
    case ::Smp::PrimitiveTypeKind::PTK_UInt8:
        return py::cast(value.operator ::Smp::UInt8());
    case ::Smp::PrimitiveTypeKind::PTK_UInt16:
        return py::cast(value.operator ::Smp::UInt16());
    case ::Smp::PrimitiveTypeKind::PTK_UInt32:
        return py::cast(value.operator ::Smp::UInt32());
    case ::Smp::PrimitiveTypeKind::PTK_UInt64:
        return py::cast(value.operator ::Smp::UInt64());
    case ::Smp::PrimitiveTypeKind::PTK_DateTime:
        return py::cast(value.operator ::Smp::DateTime());
    case ::Smp::PrimitiveTypeKind::PTK_Duration:
        return py::cast(value.operator ::Smp::Duration());
    case ::Smp::PrimitiveTypeKind::PTK_Float32:
        return py::cast(value.operator ::Smp::Float32());
    case ::Smp::PrimitiveTypeKind::PTK_Float64:
        return py::cast(value.operator ::Smp::Float64());
    default:
        return py::none();
    }
}

::Smp::AnySimple convert(const py::handle &handle,
        ::Smp::PrimitiveTypeKind kind) {

    try {

        auto value = handle.cast<const ::Smp::ISimpleField&>().GetValue();
        switch (value.GetType()) {
        case ::Smp::PrimitiveTypeKind::PTK_String8:
            return {kind, value.operator ::Smp::String8()};
        case ::Smp::PrimitiveTypeKind::PTK_Char8:
            return {kind, value.operator ::Smp::Char8()};
        case ::Smp::PrimitiveTypeKind::PTK_Bool:
            return {kind, value.operator ::Smp::Bool()};
        case ::Smp::PrimitiveTypeKind::PTK_Int8:
            return {kind, value.operator ::Smp::Int8()};
        case ::Smp::PrimitiveTypeKind::PTK_Int16:
            return {kind, value.operator ::Smp::Int16()};
        case ::Smp::PrimitiveTypeKind::PTK_Int32:
            return {kind, value.operator ::Smp::Int32()};
        case ::Smp::PrimitiveTypeKind::PTK_Int64:
            return {kind, value.operator ::Smp::Int64()};
        case ::Smp::PrimitiveTypeKind::PTK_UInt8:
            return {kind, value.operator ::Smp::UInt8()};
        case ::Smp::PrimitiveTypeKind::PTK_UInt16:
            return {kind, value.operator ::Smp::UInt16()};
        case ::Smp::PrimitiveTypeKind::PTK_UInt32:
            return {kind, value.operator ::Smp::UInt32()};
        case ::Smp::PrimitiveTypeKind::PTK_UInt64:
            return {kind, value.operator ::Smp::UInt64()};
        case ::Smp::PrimitiveTypeKind::PTK_DateTime:
            return {kind, value.operator ::Smp::DateTime()};
        case ::Smp::PrimitiveTypeKind::PTK_Duration:
            return {kind, value.operator ::Smp::Duration()};
        case ::Smp::PrimitiveTypeKind::PTK_Float32:
            return {kind, value.operator ::Smp::Float32()};
        case ::Smp::PrimitiveTypeKind::PTK_Float64:
            return {kind, value.operator ::Smp::Float64()};
        default:
            return {};
        }
    }
    catch (py::cast_error&) {
        //ignore
    }
    switch (kind) {
    case ::Smp::PrimitiveTypeKind::PTK_String8:
        return {kind, handle.cast<std::string>().c_str()}; // TODO if it is an ::Smp::IObject, return the path !
    case ::Smp::PrimitiveTypeKind::PTK_Char8:
        return {kind, handle.cast<::Smp::Char8>()};
    case ::Smp::PrimitiveTypeKind::PTK_Bool:
        return {kind, handle.cast<::Smp::Bool>()};
    case ::Smp::PrimitiveTypeKind::PTK_Int8:
        return {kind, handle.cast<::Smp::Int8>()};
    case ::Smp::PrimitiveTypeKind::PTK_Int16:
        return {kind, handle.cast<::Smp::Int16>()};
    case ::Smp::PrimitiveTypeKind::PTK_Int32:
        return {kind, handle.cast<::Smp::Int32>()};
    case ::Smp::PrimitiveTypeKind::PTK_Int64:
        return {kind, handle.cast<::Smp::Int64>()};
    case ::Smp::PrimitiveTypeKind::PTK_UInt8:
        return {kind, handle.cast<::Smp::UInt8>()};
    case ::Smp::PrimitiveTypeKind::PTK_UInt16:
        return {kind, handle.cast<::Smp::UInt16>()};
    case ::Smp::PrimitiveTypeKind::PTK_UInt32:
        return {kind, handle.cast<::Smp::UInt32>()};
    case ::Smp::PrimitiveTypeKind::PTK_UInt64:
        return {kind, handle.cast<::Smp::UInt64>()};
    case ::Smp::PrimitiveTypeKind::PTK_DateTime:
        return {kind, handle.cast<::Smp::DateTime>()};
    case ::Smp::PrimitiveTypeKind::PTK_Duration:
        return {kind, handle.cast<::Smp::Duration>()};
    case ::Smp::PrimitiveTypeKind::PTK_Float32:
        return {kind, handle.cast<::Smp::Float32>()};
    case ::Smp::PrimitiveTypeKind::PTK_Float64:
        return {kind, handle.cast<::Smp::Float64>()};
    case ::Smp::PrimitiveTypeKind::PTK_None:
        return {};
    }
    return {};
}
::Smp::UInt64 GetIndex(::Smp::Int64 index, ::Smp::UInt64 size) {
    ::Smp::Int64 result =
            index < 0 ? static_cast<::Smp::Int64>(size) + index : index;
    if (result < 0 || static_cast<::Smp::UInt64>(index) >= size)
        throw py::index_error(std::to_string(index));
    return static_cast<::Smp::UInt64>(result);
}
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

