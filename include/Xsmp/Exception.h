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

#ifndef XSMP_EXCEPTION_H_
#define XSMP_EXCEPTION_H_

#include <Smp/AnySimple.h>
#include <Smp/ComponentStateKind.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/EventId.h>
#include <Smp/SimulatorStateKind.h>
#include <sstream>
#include <string>
#include <string_view>
#include <typeinfo>

namespace Smp {
class IObject;
class IField;
class IDataflowField;
class IEventSink;
class IEventSource;
class IEntryPoint;
class IComponent;
class IOperation;
class IContainer;
class IReference;
class IArrayField;
class ISimpleArrayField;
class IFactory;

namespace Publication {
class IType;
} // namespace Publication
} // namespace Smp

namespace Xsmp::Exception {

namespace detail {

std::ostream &operator<<(std::ostream &os, const ::Smp::IObject *obj);

template <typename... Args> inline std::string FormatString(Args &&...args) {
  std::ostringstream ss;
  ((ss << args), ...);
  return ss.str();
}

[[noreturn]] void throwException(const ::Smp::IObject *sender,
                                 std::string_view name,
                                 std::string_view description,
                                 std::string_view message);
} // namespace detail

template <typename... Args>
[[noreturn]] void
throwException(const ::Smp::IObject *sender, std::string_view name,
               std::string_view description, Args &&...message) {
  detail::throwException(sender, name, description,
                         detail::FormatString(std::forward<Args>(message)...));
}

[[noreturn]] void
throwFieldAlreadyConnected(const ::Smp::IObject *sender,
                           const ::Smp::IDataflowField *source,
                           const ::Smp::IField *target);

[[noreturn]] void throwInvalidTarget(const ::Smp::IObject *sender,
                                     const ::Smp::IDataflowField *source,
                                     const ::Smp::IField *target);

[[noreturn]] void
throwCannotStore(const ::Smp::IObject *sender,
                 std::string_view msg = "The component can't be stored");

[[noreturn]] void
throwCannotRestore(const ::Smp::IObject *sender,
                   std::string_view msg = "The component can't be restored");

[[noreturn]] void throwInvalidObjectName(const ::Smp::IObject *sender,
                                         ::Smp::String8 name,
                                         std::string_view description);

[[noreturn]] void throwContainerFull(const ::Smp::IContainer *sender);

[[noreturn]] void throwDuplicateName(const ::Smp::IObject *sender,
                                     ::Smp::String8 duplicateName,
                                     const ::Smp::IObject *collection);

[[noreturn]] void throwNotContained(const ::Smp::IContainer *container,
                                    ::Smp::IComponent *component);

[[noreturn]] void throwCannotDelete(const ::Smp::IContainer *container,
                                    ::Smp::IComponent *component);

[[noreturn]] void
throwInvalidComponentState(const ::Smp::IComponent *sender,
                           ::Smp::ComponentStateKind invalidState,
                           ::Smp::ComponentStateKind expectedState);

[[noreturn]] void throwInvalidObjectType(const ::Smp::IObject *sender,
                                         ::Smp::IObject *invalidObject,
                                         const std::type_info &expectedType);

template <typename T>
[[noreturn]] void throwInvalidObjectType(const ::Smp::IObject *sender,
                                         ::Smp::IObject *invalidObject) {
  throwInvalidObjectType(sender, invalidObject, typeid(T));
}

[[noreturn]] void throwNotReferenced(const ::Smp::IReference *sender,
                                     ::Smp::IComponent *component);

[[noreturn]] void throwReferenceFull(const ::Smp::IReference *sender,
                                     ::Smp::Int64 referenceSize);

[[noreturn]] void throwCannotRemove(const ::Smp::IReference *sender,
                                    ::Smp::IComponent *component,
                                    ::Smp::Int64 lowerLimit);

[[noreturn]] void throwInvalidEventSink(const ::Smp::IObject *sender,
                                        const ::Smp::IEventSource *eventSource,
                                        const ::Smp::IEventSink *eventSink);

[[noreturn]] void
throwEventSinkAlreadySubscribed(const ::Smp::IObject *sender,
                                const ::Smp::IEventSource *eventSource,
                                const ::Smp::IEventSink *eventSink);

[[noreturn]] void
throwEventSinkNotSubscribed(const ::Smp::IObject *sender,
                            const ::Smp::IEventSource *eventSource,
                            const ::Smp::IEventSink *eventSink);

[[noreturn]] void throwInvalidOperationName(const ::Smp::IObject *sender,
                                            ::Smp::String8 operationName);

[[noreturn]] void
throwInvalidParameterCount(const ::Smp::IOperation *sender,
                           ::Smp::Int32 requestedNbParameters);

[[noreturn]] void throwInvalidParameterType(
    const ::Smp::IObject *sender, std::string_view operationName,
    std::string_view parameterName, ::Smp::PrimitiveTypeKind invalidType,
    ::Smp::PrimitiveTypeKind expectedType);

[[noreturn]] void throwInvalidArrayIndex(const ::Smp::IArrayField *sender,
                                         ::Smp::UInt64 invalidIndex);

[[noreturn]] void throwInvalidArrayIndex(const ::Smp::ISimpleArrayField *sender,
                                         ::Smp::UInt64 invalidIndex);

[[noreturn]] void throwInvalidArrayIndex(const ::Smp::IObject *sender,
                                         ::Smp::UInt64 arraySize,
                                         ::Smp::UInt64 invalidIndex);

[[noreturn]] void
throwInvalidFieldValue(const ::Smp::IField *sender,
                       const ::Smp::AnySimple &invalidFieldValue);

[[noreturn]] void throwInvalidArrayValue(const ::Smp::ISimpleArrayField *sender,
                                         ::Smp::UInt64 index,
                                         const ::Smp::AnySimple &invalidValue);

[[noreturn]] void throwInvalidArraySize(const ::Smp::ISimpleArrayField *sender,
                                        ::Smp::UInt64 invalidSize);

[[noreturn]] void throwInvalidEventName(const ::Smp::IObject *sender,
                                        ::Smp::String8 eventName);

[[noreturn]] void throwInvalidEventId(const ::Smp::IObject *sender,
                                      ::Smp::Services::EventId eventId);

[[noreturn]] void throwInvalidCycleTime(const ::Smp::IObject *sender,
                                        ::Smp::Duration cycleTime);

[[noreturn]] void throwInvalidEventTime(const ::Smp::IObject *sender,
                                        ::Smp::Duration eventTime,
                                        ::Smp::Duration currentTime);

[[noreturn]] void
throwEntryPointNotSubscribed(const ::Smp::IObject *sender,
                             const ::Smp::IEntryPoint *entryPoint,
                             std::string_view eventName);

[[noreturn]] void
throwEntryPointAlreadySubscribed(const ::Smp::IObject *sender,
                                 const ::Smp::IEntryPoint *entryPoint,
                                 std::string_view eventName);

[[noreturn]] void throwInvalidFieldName(const ::Smp::IObject *sender,
                                        ::Smp::String8 invalidFieldName);

[[noreturn]] void throwTypeNotRegistered(const ::Smp::IObject *sender,
                                         ::Smp::Uuid uuid);

[[noreturn]] void throwInvalidParameterIndex(const ::Smp::IObject *sender,
                                             ::Smp::Int32 index,
                                             ::Smp::Int32 count);

[[noreturn]] void throwVoidOperation(const ::Smp::IObject *sender);

[[noreturn]] void throwInvalidAnyType(const ::Smp::IObject *sender,
                                      ::Smp::PrimitiveTypeKind expectedType,
                                      ::Smp::PrimitiveTypeKind invalidType);

[[noreturn]] void throwInvalidReturnValue(const ::Smp::IObject *sender,
                                          const ::Smp::AnySimple &value);

[[noreturn]] void throwInvalidParameterValue(const ::Smp::IObject *sender,
                                             std::string_view parameterName,
                                             const ::Smp::AnySimple &value);

[[noreturn]] void throwInvalidFieldType(const ::Smp::IObject *sender,
                                        const ::Smp::Uuid &uuid);
[[noreturn]] void throwInvalidFieldType(const ::Smp::IObject *sender,
                                        const ::Smp::Publication::IType *type);
[[noreturn]] void throwInvalidFieldType(const ::Smp::IObject *sender,
                                        ::Smp::PrimitiveTypeKind kind);

[[noreturn]] void throwDuplicateUuid(const ::Smp::IObject *sender,
                                     const ::Smp::IFactory *oldFactory,
                                     ::Smp::String8 newName);

[[noreturn]] void throwLibraryNotFound(const ::Smp::IObject *sender,
                                       ::Smp::String8 libraryName,
                                       std::string_view msg = "");

[[noreturn]] void throwInvalidLibrary(const ::Smp::IObject *sender,
                                      ::Smp::String8 libraryName,
                                      std::string_view msg);

[[noreturn]] void throwInvalidSimulationTime(const ::Smp::IObject *sender,
                                             ::Smp::Duration current,
                                             ::Smp::Duration provided,
                                             ::Smp::Duration max);

[[noreturn]] void
throwTypeAlreadyRegistered(const ::Smp::IObject *sender,
                           ::Smp::String8 newTypeName,
                           const ::Smp::Publication::IType *existingType);

[[noreturn]] void throwInvalidPrimitiveType(const ::Smp::IObject *sender,
                                            ::Smp::String8 typeName,
                                            ::Smp::PrimitiveTypeKind kind);

[[noreturn]] void throwInvalidSimulatorState(const ::Smp::IObject *sender,
                                             ::Smp::SimulatorStateKind state);

[[noreturn]] void throwDuplicateLiteral(const ::Smp::IObject *sender,
                                        ::Smp::String8 literalName,
                                        ::Smp::Int32 literalValue);

} // namespace Xsmp::Exception

#endif // XSMP_EXCEPTION_H_
