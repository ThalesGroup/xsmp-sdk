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

#ifndef PYTHON_ECSS_SMP_H_
#define PYTHON_ECSS_SMP_H_

#include <pybind11/pybind11.h>


#include <Smp/AnySimple.h>
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
#include <Smp/IOperation.h>
#include <Smp/IProperty.h>
#include <Smp/IReference.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimulator.h>
#include <Smp/IStructureField.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ILinkRegistry.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/IResolver.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Xsmp/DateTime.h>
#include <Xsmp/Duration.h>
#include <Xsmp/Helper.h>
#include <algorithm>
#include <memory>
#include <string>
#include <type_traits>
#include <typeinfo>
#include <vector>
#include <sstream>

namespace py = pybind11;

namespace PYBIND11_NAMESPACE {
struct TypeHierarchy {

    template<typename T>
    static TypeHierarchy of(std::vector<TypeHierarchy> _derived = { }) {
        return TypeHierarchy { typeid(T), [](void *src) -> void* {
            return dynamic_cast<T*>(reinterpret_cast<::Smp::IObject*>(src));
        },
        [](::Smp::IObject *src) -> bool {
            return dynamic_cast<T*>(src);
        },
        std::move(_derived) };
    }
    const std::type_info &base;
    void* (*caster)(void*);
    bool (*isInstance)(::Smp::IObject*);
    std::vector<TypeHierarchy> derived;
};

static inline const TypeHierarchy IObjectHierarchy =
        TypeHierarchy::template of<::Smp::IObject>(
                {
//IObject
                        /*TypeHierarchy::template of<::Smp::ICollection>( { }),*/
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

                        //TypeHierarchy::template of<::Smp::IFactory>( ),

                        TypeHierarchy::template of<::Smp::IOperation>(),

                        //TypeHierarchy::template of<::Smp::IParameter>( ),

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

class SmpClass: public detail::generic_type {

public:
    using type = ::Smp::IObject;
    using holder_type = std::unique_ptr<type>;
    explicit SmpClass(type *obj) {

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

        processHierarchy(IObjectHierarchy, obj, record);

        detail::generic_type::initialize(record);

    }

    static bool processHierarchy(const TypeHierarchy &hierarchy, type *object,
            detail::type_record &rec) {
        bool ignore = false;

        for (const auto &elem : hierarchy.derived) {
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

void Register(::Smp::IObject *object) {
    if (detail::get_local_type_info(typeid(*object)) == nullptr) {
        SmpClass(object)

        .doc() = "Automatic Python binding for '"
                + Xsmp::Helper::TypeName(object) + "'.";

    }

}

} // namespace PYBIND11_NAMESPACE



py::object convert(::Smp::AnySimple value) {
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

::Smp::AnySimple convert(py::handle handle, ::Smp::PrimitiveTypeKind kind) {

    try {
        const auto &field = handle.cast<::Smp::ISimpleField&>();
        handle = convert(field.GetValue());
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
inline ::Smp::UInt64 GetIndex(::Smp::Int64 index, ::Smp::UInt64 size) {
    ::Smp::Int64 result =
            index < 0 ? static_cast<::Smp::Int64>(size) + index : index;
    if (result < 0 || static_cast<::Smp::UInt64>(index) >= size)
        throw py::index_error(std::to_string(index));
    return static_cast<::Smp::UInt64>(result);
}

#endif // PYTHON_ECSS_SMP_H_
