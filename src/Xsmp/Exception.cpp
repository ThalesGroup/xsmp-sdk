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

#include <Smp/AnySimple.h>
#include <Smp/CannotDelete.h>
#include <Smp/CannotRemove.h>
#include <Smp/CannotRestore.h>
#include <Smp/CannotStore.h>
#include <Smp/ComponentStateKind.h>
#include <Smp/ContainerFull.h>
#include <Smp/DuplicateName.h>
#include <Smp/DuplicateUuid.h>
#include <Smp/EventSinkAlreadySubscribed.h>
#include <Smp/EventSinkNotSubscribed.h>
#include <Smp/Exception.h>
#include <Smp/FieldAlreadyConnected.h>
#include <Smp/IArrayField.h>
#include <Smp/IComponent.h>
#include <Smp/IContainer.h>
#include <Smp/IDataflowField.h>
#include <Smp/IEntryPoint.h>
#include <Smp/IEventSink.h>
#include <Smp/IEventSource.h>
#include <Smp/IFactory.h>
#include <Smp/IOperation.h>
#include <Smp/IReference.h>
#include <Smp/ISimpleArrayField.h>
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
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/DuplicateLiteral.h>
#include <Smp/Publication/IType.h>
#include <Smp/Publication/InvalidPrimitiveType.h>
#include <Smp/Publication/TypeAlreadyRegistered.h>
#include <Smp/Publication/TypeNotRegistered.h>
#include <Smp/ReferenceFull.h>
#include <Smp/Services/EntryPointAlreadySubscribed.h>
#include <Smp/Services/EntryPointNotSubscribed.h>
#include <Smp/Services/EventId.h>
#include <Smp/Services/InvalidCycleTime.h>
#include <Smp/Services/InvalidEventId.h>
#include <Smp/Services/InvalidEventName.h>
#include <Smp/Services/InvalidEventTime.h>
#include <Smp/Services/InvalidSimulationTime.h>
#include <Smp/SimulatorStateKind.h>
#include <Smp/Uuid.h>
#include <Smp/VoidOperation.h>
#include <Xsmp/Duration.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <ostream>
#include <string>
#include <string_view>
#include <typeinfo>

namespace Xsmp::Exception {

class Exception : public virtual ::Smp::Exception {
public:
  template <typename... Args>
  Exception(const ::Smp::IObject *sender, std::string_view name,
            std::string_view description, Args &&...args) noexcept
      : _sender(sender), _name(name), _description(description),
        _message(
            Xsmp::Exception::detail::FormatString(std::forward<Args>(args)...)),
        _whatStr(_name + "(" + _description + "): " + _message) {}
  ~Exception() noexcept override = default;
  Exception(const Exception &) = default;
  Exception &operator=(const Exception &) = default;

  const ::Smp::Char8 *what() const noexcept final { return _whatStr.c_str(); }
  ::Smp::String8 GetName() const noexcept final { return _name.c_str(); }
  ::Smp::String8 GetDescription() const noexcept final {
    return _description.c_str();
  }
  ::Smp::String8 GetMessage() const noexcept final { return _message.c_str(); }
  const ::Smp::IObject *GetSender() const noexcept final { return _sender; }

protected:
  static ::Smp::String8 nullCheck(::Smp::String8 str) {
    return str ? str : "<null>";
  }

private:
  const ::Smp::IObject *_sender;
  std::string _name;
  std::string _description;
  std::string _message;
  std::string _whatStr;
};

class FieldAlreadyConnected final : public Exception,
                                    public ::Smp::FieldAlreadyConnected {
public:
  FieldAlreadyConnected(const ::Smp::IObject *sender,
                        const ::Smp::IDataflowField *source,
                        const ::Smp::IField *target)
      : Exception(sender, __func__,
                  "Cannot connect a target field to a data flow field that is "
                  "already connected",

                  "Fields '", target, "' and '", source,
                  "' are already connected."),

        _source(source), _target(target) {}

  ~FieldAlreadyConnected() noexcept override = default;
  FieldAlreadyConnected(const FieldAlreadyConnected &) = default;
  FieldAlreadyConnected &operator=(const FieldAlreadyConnected &) = default;

  const ::Smp::IDataflowField *GetSource() const noexcept override {
    return _source;
  }

  const ::Smp::IField *GetTarget() const noexcept override { return _target; }

private:
  const ::Smp::IDataflowField *_source;
  const ::Smp::IField *_target;
};

class InvalidTarget final : public Exception, public ::Smp::InvalidTarget {
public:
  InvalidTarget(const ::Smp::IObject *sender,
                const ::Smp::IDataflowField *source,
                const ::Smp::IField *target)
      : Exception(sender, __func__,
                  "Cannot connect two data flow fields of incompatible types",

                  "The type of the target '", target,
                  "' is not compatible with '", source, "'."),

        _source(source), _target(target) {}

  ~InvalidTarget() noexcept override = default;
  InvalidTarget(const InvalidTarget &) = default;
  InvalidTarget &operator=(const InvalidTarget &) = default;

  const ::Smp::IDataflowField *GetSource() const noexcept override {
    return _source;
  }

  const ::Smp::IField *GetTarget() const noexcept override { return _target; }

private:
  const ::Smp::IDataflowField *_source;
  const ::Smp::IField *_target;
};

class CannotStore final : public Exception, public ::Smp::CannotStore {
public:
  CannotStore(const ::Smp::IObject *sender, std::string_view msg)
      : Exception(sender, __func__,
                  "This exception is raised when the component cannot store "
                  "its data to the storage writer given to the Store() method",
                  msg) {}
  ~CannotStore() noexcept override = default;
  CannotStore(const CannotStore &) = default;
  CannotStore &operator=(const CannotStore &) = default;
};

class CannotRestore final : public Exception, public ::Smp::CannotRestore {
public:
  CannotRestore(const ::Smp::IObject *sender, std::string_view msg)
      : Exception(sender, __func__,
                  "This exception is raised when the content of the storage "
                  "reader passed to the Restore() method contains invalid data",
                  msg) {}
  ~CannotRestore() noexcept override = default;
  CannotRestore(const CannotRestore &) = default;
  CannotRestore &operator=(const CannotRestore &) = default;
};

class InvalidObjectName final : public Exception,
                                public ::Smp::InvalidObjectName {
public:
  InvalidObjectName(const ::Smp::IObject *sender, ::Smp::String8 name,
                    std::string_view description)
      : Exception(sender, __func__,
                  "Cannot set an object's name to an invalid name",
                  "The object's name '", nullCheck(name), "' is invalid. ",
                  description),

        _name(nullCheck(name)) {}
  ~InvalidObjectName() noexcept override = default;
  InvalidObjectName(const InvalidObjectName &) = default;
  InvalidObjectName &operator=(const InvalidObjectName &) = default;

  ::Smp::String8 GetInvalidName() const noexcept override {
    return _name.c_str();
  }

private:
  std::string _name;
};

class ContainerFull final : public Exception, public ::Smp::ContainerFull {
public:
  explicit ContainerFull(const ::Smp::IContainer *sender)
      : Exception(sender, __func__,
                  "Cannot add a component to a container that is full, i.e. "
                  "where the Count has reached the Upper limit",
                  "The Container '", sender, "' is full, upper limit is '",
                  sender->GetCount(), "'."),

        _containerSize(sender->GetCount()) {}
  ~ContainerFull() noexcept override = default;
  ContainerFull(const ContainerFull &) = default;
  ContainerFull &operator=(const ContainerFull &) = default;

  ::Smp::String8 GetContainerName() const noexcept override {
    return GetSender()->GetName();
  }

  ::Smp::Int64 GetContainerSize() const noexcept override {
    return _containerSize;
  }

private:
  ::Smp::Int64 _containerSize;
};

class DuplicateName final : public Exception, public ::Smp::DuplicateName {
public:
  DuplicateName(const ::Smp::IObject *sender, ::Smp::String8 duplicateName,
                const ::Smp::IObject *collection)
      : Exception(sender, __func__,
                  "Cannot add an object to a collection of objects, which have "
                  "to have unique names, but another object with the same name "
                  "does exist already in this collection. This would lead to "
                  "duplicate names",
                  "Tried to add an object named '", nullCheck(duplicateName),
                  "' in '", collection,
                  "' collection already containing an object with this name."),

        _duplicateName(nullCheck(duplicateName)) {}

  ~DuplicateName() noexcept override = default;
  DuplicateName(const DuplicateName &) = default;
  DuplicateName &operator=(const DuplicateName &) = default;

  ::Smp::String8 GetDuplicateName() const noexcept override {
    return _duplicateName.c_str();
  }

private:
  std::string _duplicateName;
};

class NotContained final : public Exception, public ::Smp::NotContained {
public:
  NotContained(const ::Smp::IContainer *container, ::Smp::IComponent *component)
      : Exception(container, __func__,
                  "This exception is thrown when trying to delete a component "
                  "from a container which was not contained before",

                  "Cannot delete component '", component,
                  "' from the container '", container, "'."),

        _component(component) {}
  ~NotContained() noexcept override = default;
  NotContained(const NotContained &) = default;
  NotContained &operator=(const NotContained &) = default;

  ::Smp::String8 GetContainerName() const noexcept override {
    return GetSender()->GetName();
  }

  ::Smp::IComponent *GetComponent() const noexcept override {
    return _component;
  }

private:
  ::Smp::IComponent *_component;
};

class CannotDelete final : public Exception, public ::Smp::CannotDelete {
public:
  CannotDelete(const ::Smp::IContainer *container, ::Smp::IComponent *component)
      : Exception(container, __func__,
                  "This exception is thrown when trying to delete a component "
                  "from a container when the number of contained components is "
                  "lower than or equal to the Lower limit",

                  "Tried to delete ", component, " component from ", container,
                  " container, but the number of contained components is lower "
                  "than or equal to the Lower limit: ",
                  container->GetLower()),

        _component(component), _lowerLimit(container->GetLower()) {}
  ~CannotDelete() noexcept override = default;
  CannotDelete(const CannotDelete &) = default;
  CannotDelete &operator=(const CannotDelete &) = default;

  ::Smp::String8 GetContainerName() const noexcept override {
    return GetSender()->GetName();
  }

  ::Smp::IComponent *GetComponent() const noexcept override {
    return _component;
  }

  ::Smp::Int64 GetLowerLimit() const noexcept override { return _lowerLimit; }

private:
  ::Smp::IComponent *_component;
  ::Smp::Int64 _lowerLimit;
};

class InvalidComponentState final : public Exception,
                                    public ::Smp::InvalidComponentState {
public:
  InvalidComponentState(const ::Smp::IComponent *sender,
                        ::Smp::ComponentStateKind invalidState,
                        ::Smp::ComponentStateKind expectedState)
      : Exception(sender, __func__,
                  "This exception is raised by a component when one of its "
                  "methods is called in an invalid state",

                  "The ComponentState of ", sender, " is in ", invalidState,
                  " state instead of ", expectedState, " state."),

        _invalidState(invalidState), _expectedState(expectedState) {}
  ~InvalidComponentState() noexcept override = default;
  InvalidComponentState(const InvalidComponentState &) = default;
  InvalidComponentState &operator=(const InvalidComponentState &) = default;

  ::Smp::ComponentStateKind GetInvalidState() const noexcept override {
    return _invalidState;
  }

  ::Smp::ComponentStateKind GetExpectedState() const noexcept override {
    return _expectedState;
  }

private:
  ::Smp::ComponentStateKind _invalidState;
  ::Smp::ComponentStateKind _expectedState;
};

class InvalidObjectType final : public Exception,
                                public ::Smp::InvalidObjectType {
public:
  InvalidObjectType(const ::Smp::IObject *sender, ::Smp::IObject *invalidObject,
                    const std::type_info &expectedType)
      : Exception(sender, __func__, "Cannot pass an object of wrong type",

                  invalidObject, " of type ",
                  ::Xsmp::Helper::TypeName(invalidObject),
                  " cannot be casted to ",
                  Xsmp::Helper::demangle(expectedType.name()), "."),

        _invalidObject(invalidObject) {}

  ~InvalidObjectType() noexcept override = default;
  InvalidObjectType(const InvalidObjectType &) = default;
  InvalidObjectType &operator=(const InvalidObjectType &) = default;

  ::Smp::IObject *GetInvalidObject() const noexcept override {
    return _invalidObject;
  }

private:
  ::Smp::IObject *_invalidObject;
};

class NotReferenced final : public Exception, public ::Smp::NotReferenced {
public:
  NotReferenced(const ::Smp::IReference *reference,
                ::Smp::IComponent *component)
      : Exception(reference, __func__,
                  "This exception is thrown when trying to remove a component "
                  "from a reference which was not referenced before",

                  "Tried to remove ", component,
                  " component from the reference ", reference,
                  " which was not referenced before."),

        _component(component) {}

  ~NotReferenced() noexcept override = default;
  NotReferenced(const NotReferenced &) = default;
  NotReferenced &operator=(const NotReferenced &) = default;

  ::Smp::String8 GetReferenceName() const noexcept override {
    return GetSender()->GetName();
  }

  ::Smp::IComponent *GetComponent() const noexcept override {
    return _component;
  }

private:
  ::Smp::IComponent *_component;
};

class ReferenceFull final : public Exception, public ::Smp::ReferenceFull {
public:
  ReferenceFull(const ::Smp::IReference *sender, ::Smp::Int64 referenceSize)
      : Exception(sender, __func__,
                  "Cannot add a component to a reference that is full, i.e. "
                  "where the Count has reached the Upper limit",

                  "Tried to add a component to ", sender,
                  " that is full (max size: ", referenceSize, ")"),

        _referenceSize(referenceSize) {}

  ~ReferenceFull() noexcept override = default;
  ReferenceFull(const ReferenceFull &) = default;
  ReferenceFull &operator=(const ReferenceFull &) = default;

  ::Smp::String8 GetReferenceName() const noexcept override {
    return GetSender()->GetName();
  }

  ::Smp::Int64 GetReferenceSize() const noexcept override {
    return _referenceSize;
  }

private:
  ::Smp::Int64 _referenceSize;
};

/// This exception is thrown when trying to remove a component from a
/// reference when the number of referenced components is lower than or
/// equal to the Lower limit.
class CannotRemove final : public Exception, public ::Smp::CannotRemove {
public:
  CannotRemove(const ::Smp::IReference *sender, ::Smp::IComponent *component,
               ::Smp::Int64 lowerLimit)
      : Exception(sender, __func__,
                  "This exception is thrown when trying to remove a component "
                  "from a reference when the number of referenced components "
                  "is lower than or equal to the Lower limit",

                  "Tried to remove ", component, " component from ", sender,
                  " reference, but the number of referenced components is "
                  "lower than or equal to the Lower limit: ",
                  lowerLimit),

        _component(component), _lowerLimit(lowerLimit) {}

  ~CannotRemove() noexcept override = default;
  CannotRemove(const CannotRemove &) = default;
  CannotRemove &operator=(const CannotRemove &) = default;

  ::Smp::String8 GetReferenceName() const noexcept override {
    return GetSender()->GetName();
  }

  ::Smp::IComponent *GetComponent() const noexcept override {
    return _component;
  }

  ::Smp::Int64 GetLowerLimit() const noexcept override { return _lowerLimit; }

private:
  ::Smp::IComponent *_component;
  ::Smp::Int64 _lowerLimit;
};

class InvalidEventSink final : public Exception,
                               public ::Smp::InvalidEventSink {
public:
  InvalidEventSink(const ::Smp::IObject *sender,
                   const ::Smp::IEventSource *eventSource,
                   const ::Smp::IEventSink *eventSink)
      : Exception(sender, __func__,
                  "Cannot subscribe an event sink to an event source that has "
                  "a different event type",

                  "Tried to subscribe ", eventSource, " event source to ",
                  eventSink, " event sink which has a different type."),

        _eventSource(eventSource), _eventSink(eventSink) {}

  ~InvalidEventSink() noexcept override = default;
  InvalidEventSink(const InvalidEventSink &) = default;
  InvalidEventSink &operator=(const InvalidEventSink &) = default;

  const ::Smp::IEventSource *GetEventSource() const noexcept override {
    return _eventSource;
  }

  const ::Smp::IEventSink *GetEventSink() const noexcept override {
    return _eventSink;
  }

private:
  const ::Smp::IEventSource *_eventSource;
  const ::Smp::IEventSink *_eventSink;
};

class EventSinkAlreadySubscribed final
    : public Exception,
      public ::Smp::EventSinkAlreadySubscribed {
public:
  EventSinkAlreadySubscribed(const ::Smp::IObject *sender,
                             const ::Smp::IEventSource *eventSource,
                             const ::Smp::IEventSink *eventSink)
      : Exception(sender, __func__,
                  "Cannot subscribe an event sink to an event source that is "
                  "already subscribed",

                  "Tried to subscribe ", eventSource, " event source to ",
                  eventSink, " event sink that is already subscribed."),

        _eventSource(eventSource), _eventSink(eventSink) {}

  ~EventSinkAlreadySubscribed() noexcept override = default;
  EventSinkAlreadySubscribed(const EventSinkAlreadySubscribed &) = default;
  EventSinkAlreadySubscribed &
  operator=(const EventSinkAlreadySubscribed &) = default;

  const ::Smp::IEventSource *GetEventSource() const noexcept override {
    return _eventSource;
  }

  const ::Smp::IEventSink *GetEventSink() const noexcept override {
    return _eventSink;
  }

private:
  const ::Smp::IEventSource *_eventSource;
  const ::Smp::IEventSink *_eventSink;
};

class EventSinkNotSubscribed final : public Exception,
                                     public ::Smp::EventSinkNotSubscribed {
public:
  EventSinkNotSubscribed(const ::Smp::IObject *sender,
                         const ::Smp::IEventSource *eventSource,
                         const ::Smp::IEventSink *eventSink)
      : Exception(sender, __func__,
                  "Cannot unsubscribe an event sink from an event source that "
                  "is not subscribed to it",
                  "Tried to unsubscribe ", eventSink, " event sink from ",
                  eventSource, " event source that is not subscribed to it."),

        _eventSource(eventSource), _eventSink(eventSink) {}

  ~EventSinkNotSubscribed() noexcept override = default;
  EventSinkNotSubscribed(const EventSinkNotSubscribed &) = default;
  EventSinkNotSubscribed &operator=(const EventSinkNotSubscribed &) = default;

  const ::Smp::IEventSource *GetEventSource() const noexcept override {
    return _eventSource;
  }

  const ::Smp::IEventSink *GetEventSink() const noexcept override {
    return _eventSink;
  }

private:
  const ::Smp::IEventSource *_eventSource;
  const ::Smp::IEventSink *_eventSink;
};

class InvalidOperationName final : public Exception,
                                   public ::Smp::InvalidOperationName {
public:
  InvalidOperationName(const ::Smp::IObject *sender,
                       ::Smp::String8 operationName)
      : Exception(sender, __func__,
                  "This exception is raised by the Invoke() method when trying "
                  "to invoke a method that does not exist, or that does not "
                  "support dynamic invocation",

                  "Tried to invoke a method that does not exist, or that does "
                  "not support dynamic invocation in operation: ",
                  nullCheck(operationName)),

        _operationName(nullCheck(operationName)) {}

  ~InvalidOperationName() noexcept override = default;
  InvalidOperationName(const InvalidOperationName &) = default;
  InvalidOperationName &operator=(const InvalidOperationName &) = default;

  ::Smp::String8 GetOperationName() const noexcept override {
    return _operationName.c_str();
  }

private:
  std::string _operationName;
};

class InvalidParameterCount final : public Exception,
                                    public ::Smp::InvalidParameterCount {
public:
  InvalidParameterCount(const ::Smp::IOperation *sender,
                        ::Smp::Int32 requestedNbParameters)
      : Exception(sender, __func__,
                  "This exception is raised by the Invoke() method when trying "
                  "to invoke a method with a wrong number of parameters",
                  "Tried to invoke operation ", sender, " with ",
                  requestedNbParameters, " parameter(s) instead of ",
                  sender->GetParameters()->size(), "."),

        _rightNbOperationParameters(
            static_cast<::Smp::Int32>(sender->GetParameters()->size())),
        _requestedNbParameters(requestedNbParameters) {}
  ~InvalidParameterCount() noexcept override = default;
  InvalidParameterCount(const InvalidParameterCount &) = default;
  InvalidParameterCount &operator=(const InvalidParameterCount &) = default;

  ::Smp::String8 GetOperationName() const noexcept override {
    return GetSender()->GetName();
  }

  ::Smp::Int32 GetOperationParameters() const noexcept override {
    return _rightNbOperationParameters;
  }

  ::Smp::Int32 GetRequestParameters() const noexcept override {
    return _requestedNbParameters;
  }

private:
  ::Smp::Int32 _rightNbOperationParameters;
  ::Smp::Int32 _requestedNbParameters;
};

class InvalidParameterType final : public Exception,
                                   public ::Smp::InvalidParameterType {
public:
  InvalidParameterType(const ::Smp::IObject *sender,
                       std::string_view operationName,
                       std::string_view parameterName,
                       ::Smp::PrimitiveTypeKind invalidType,
                       ::Smp::PrimitiveTypeKind expectedType)
      : Exception(sender, __func__,
                  "This exception is raised by the Invoke() method when trying "
                  "to invoke a method passing a parameter of wrong type",
                  "In operation ", operationName, ", tried to pass parameter ",
                  parameterName, " which is of type ", invalidType,
                  " instead of expected type ", expectedType),

        _operationName(operationName), _parameterName(parameterName),
        _invalidType(invalidType), _expectedType(expectedType) {}
  ~InvalidParameterType() noexcept override = default;
  InvalidParameterType(const InvalidParameterType &) = default;
  InvalidParameterType &operator=(const InvalidParameterType &) = default;

  ::Smp::String8 GetOperationName() const noexcept override {
    return _operationName.c_str();
  }

  ::Smp::String8 GetParameterName() const noexcept override {
    return _parameterName.c_str();
  }

  ::Smp::PrimitiveTypeKind GetInvalidType() const noexcept override {
    return _invalidType;
  }

  ::Smp::PrimitiveTypeKind GetExpectedType() const noexcept override {
    return _expectedType;
  }

private:
  std::string _operationName;
  std::string _parameterName;
  ::Smp::PrimitiveTypeKind _invalidType;
  ::Smp::PrimitiveTypeKind _expectedType;
};

class InvalidArrayIndex final : public Exception,
                                public ::Smp::InvalidArrayIndex {
public:
  InvalidArrayIndex(const ::Smp::IObject *sender, ::Smp::Int64 arraySize,
                    ::Smp::Int64 invalidIndex)
      : Exception(sender, __func__,
                  "This exception is raised when an invalid index is specified",

                  "The index '", invalidIndex,
                  "' is negative or exceeds the array size '", arraySize,
                  "' of \'", sender, "\'."),

        _arraySize(arraySize), _invalidIndex(invalidIndex) {}
  ~InvalidArrayIndex() noexcept override = default;
  InvalidArrayIndex(const InvalidArrayIndex &) = default;
  InvalidArrayIndex &operator=(const InvalidArrayIndex &) = default;

  ::Smp::Int64 GetInvalidIndex() const noexcept override {
    return _invalidIndex;
  }
  ::Smp::Int64 GetArraySize() const noexcept override { return _arraySize; }

private:
  ::Smp::Int64 _arraySize;
  ::Smp::Int64 _invalidIndex;
};

class InvalidFieldValue final : public Exception,
                                public ::Smp::InvalidFieldValue {
public:
  InvalidFieldValue(const ::Smp::IField *sender,
                    const ::Smp::AnySimple &invalidFieldValue)
      : Exception(sender, __func__,
                  "This exception is raised when trying to assign an illegal "
                  "value to a field",

                  "Cannot assign value \'", invalidFieldValue,
                  "' to the field ", sender, " of ",
                  sender->GetType()->GetName(), "'s type."),

        _invalidFieldValue(invalidFieldValue) {}
  ~InvalidFieldValue() noexcept override = default;
  InvalidFieldValue(const InvalidFieldValue &) = default;
  InvalidFieldValue &operator=(const InvalidFieldValue &) = default;

  ::Smp::AnySimple GetInvalidFieldValue() const noexcept override {
    return _invalidFieldValue;
  }

private:
  ::Smp::AnySimple _invalidFieldValue;
};

class InvalidArrayValue final : public Exception,
                                public ::Smp::InvalidArrayValue {
public:
  InvalidArrayValue(const ::Smp::ISimpleArrayField *sender, ::Smp::Int64 index,
                    const ::Smp::AnySimple &invalidValue)
      : Exception(sender, __func__,
                  "This exception is raised when trying to assign an illegal "
                  "value to an array field",

                  "Cannot assign value \'", invalidValue, "' to the field ",
                  sender, "[", index, "] of ", sender->GetType()->GetName(),
                  "'s type."),

        _index(index), _invalidValue(invalidValue) {}
  ~InvalidArrayValue() noexcept override = default;
  InvalidArrayValue(const InvalidArrayValue &) = default;
  InvalidArrayValue &operator=(const InvalidArrayValue &) = default;

  ::Smp::Int64 GetInvalidValueIndex() const noexcept override { return _index; }
  ::Smp::AnySimple GetInvalidValue() const noexcept override {
    return _invalidValue;
  }

private:
  ::Smp::Int64 _index;
  ::Smp::AnySimple _invalidValue;
};

class InvalidArraySize final : public Exception,
                               public ::Smp::InvalidArraySize {
public:
  InvalidArraySize(const ::Smp::ISimpleArrayField *sender,
                   ::Smp::Int64 invalidSize)
      : Exception(
            sender, __func__,
            "This exception is raised when an invalid array size is specified",

            "The array size ", invalidSize, " of ", sender,
            " is invalid, expecting ", sender->GetSize(), " elements."),

        _arraySize(static_cast<::Smp::Int64>(sender->GetSize())),
        _invalidSize(invalidSize) {}
  ~InvalidArraySize() noexcept override = default;
  InvalidArraySize(const InvalidArraySize &) = default;
  InvalidArraySize &operator=(const InvalidArraySize &) = default;

  ::Smp::Int64 GetArraySize() const noexcept override { return _arraySize; }
  ::Smp::Int64 GetInvalidSize() const noexcept override { return _invalidSize; }

private:
  ::Smp::Int64 _arraySize;
  ::Smp::Int64 _invalidSize;
};

class InvalidEventName final : public Exception,
                               public ::Smp::Services::InvalidEventName {
public:
  explicit InvalidEventName(const ::Smp::IObject *sender,
                            ::Smp::String8 eventName)
      : Exception(
            sender, __func__,
            "This exception is thrown by the QueryEventId() method of the "
            "event manager when an empty event name has been provided",
            "The Event Name '", nullCheck(eventName), "' is invalid.") {}
  ~InvalidEventName() noexcept override = default;
  InvalidEventName(const InvalidEventName &) = default;
  InvalidEventName &operator=(const InvalidEventName &) = default;
};

class InvalidEventId final : public Exception,
                             public ::Smp::Services::InvalidEventId {
public:
  InvalidEventId(const ::Smp::IObject *sender, ::Smp::Services::EventId eventId)
      : Exception(
            sender, __func__,
            "This exception is raised when an invalid event id is provided, "
            "e.g.when calling Subscribe(), Unsubscribe()"
            " or Emit() of the Event Manager (using an invalid global event "
            "id), or when calling SetEventSimulationTime(), "
            "SetEventMissionTime(), SetEventEpochTime(), SetEventZuluTime(), "
            "SetEventCycleTime(), SetEventCount() or RemoveEvent() "
            "of the Scheduler (using an invalid scheduler event id)",
            "The EventId '", eventId, "' is invalid."),

        _eventId(eventId) {}
  ~InvalidEventId() noexcept override = default;
  InvalidEventId(const InvalidEventId &) = default;
  InvalidEventId &operator=(const InvalidEventId &) = default;

  ::Smp::Services::EventId GetInvalidEventId() const noexcept override {
    return _eventId;
  }

private:
  ::Smp::Services::EventId _eventId;
};

class InvalidCycleTime final : public Exception,
                               public ::Smp::Services::InvalidCycleTime {
public:
  InvalidCycleTime(const ::Smp::IObject *sender, ::Xsmp::Duration cycleTime)
      : Exception(sender, __func__,
                  "This exception is thrown by one of the AddEvent() methods "
                  "of the scheduler when the event "
                  "is a cyclic event (i.e. repeat is not 0),  but the cycle "
                  "time specified is not a positive duration",
                  "The cycle time '", cycleTime, "' is not a positive value.") {
  }
  ~InvalidCycleTime() noexcept override = default;
  InvalidCycleTime(const InvalidCycleTime &) = default;
  InvalidCycleTime &operator=(const InvalidCycleTime &) = default;
};

class InvalidEventTime final : public Exception,
                               public ::Smp::Services::InvalidEventTime {
public:
  InvalidEventTime(const ::Smp::IObject *sender, ::Xsmp::Duration eventTime,
                   ::Xsmp::Duration currentTime)
      : Exception(sender, __func__,
                  "This exception is thrown by one of the AddEvent() methods "
                  "of the scheduler when the time specified for the first "
                  "execution of the  event is in the past",
                  "The event time '", eventTime,
                  "' is before the current time '", currentTime, "'.") {}
  ~InvalidEventTime() noexcept override = default;
  InvalidEventTime(const InvalidEventTime &) = default;
  InvalidEventTime &operator=(const InvalidEventTime &) = default;
};
class EntryPointNotSubscribed final
    : public Exception,
      public ::Smp::Services::EntryPointNotSubscribed {
public:
  EntryPointNotSubscribed(const ::Smp::IObject *sender,
                          const ::Smp::IEntryPoint *entryPoint,
                          std::string_view eventName)
      : Exception(sender, __func__,
                  "Cannot unsubscribe an entry point from an event that is not "
                  "subscribed to it",

                  "The EntryPoint ", entryPoint, " is not subscribed to '",
                  eventName, "'."),

        _entryPoint(entryPoint), _eventName(eventName) {}
  ~EntryPointNotSubscribed() noexcept override = default;
  EntryPointNotSubscribed(const EntryPointNotSubscribed &) = default;
  EntryPointNotSubscribed &operator=(const EntryPointNotSubscribed &) = default;

  const ::Smp::IEntryPoint *GetEntryPoint() const noexcept override {
    return _entryPoint;
  }

  ::Smp::String8 GetEventName() const noexcept override {
    return _eventName.c_str();
  }

private:
  const ::Smp::IEntryPoint *_entryPoint;
  std::string _eventName;
};

class EntryPointAlreadySubscribed final
    : public Exception,
      public ::Smp::Services::EntryPointAlreadySubscribed {
public:
  EntryPointAlreadySubscribed(const ::Smp::IObject *sender,
                              const ::Smp::IEntryPoint *entryPoint,
                              std::string_view eventName)
      : Exception(sender, __func__,
                  "Cannot subscribe an entry point to an event that is already "
                  "subscribed",

                  "The EntryPoint ", entryPoint, " is already subscribed to '",
                  eventName, "'."),

        _entryPoint(entryPoint), _eventName(eventName) {}
  ~EntryPointAlreadySubscribed() noexcept override = default;
  EntryPointAlreadySubscribed(const EntryPointAlreadySubscribed &) = default;
  EntryPointAlreadySubscribed &
  operator=(const EntryPointAlreadySubscribed &) = default;

  const ::Smp::IEntryPoint *GetEntryPoint() const noexcept override {
    return _entryPoint;
  }

  ::Smp::String8 GetEventName() const noexcept override {
    return _eventName.c_str();
  }

private:
  const ::Smp::IEntryPoint *_entryPoint;
  std::string _eventName;
};

class InvalidFieldName final : public Exception,
                               public ::Smp::InvalidFieldName {
public:
  InvalidFieldName(const ::Smp::IObject *sender, ::Smp::String8 name)
      : Exception(
            sender, __func__,
            "This exception is raised when an invalid field name is specified",
            "'", sender, "' does no contains a field named '", nullCheck(name),
            "'."),

        _fieldName(nullCheck(name)) {}
  ~InvalidFieldName() noexcept override = default;
  InvalidFieldName(const InvalidFieldName &) = default;
  InvalidFieldName &operator=(const InvalidFieldName &) = default;

  ::Smp::String8 GetFieldName() const noexcept override {
    return _fieldName.c_str();
  }

private:
  std::string _fieldName;
};

class TypeNotRegistered final : public Exception,
                                public ::Smp::Publication::TypeNotRegistered {
public:
  TypeNotRegistered(const ::Smp::IObject *sender, ::Smp::Uuid uuid)
      : Exception(sender, __func__,
                  "Cannot publish a feature with a type Uuid that has not been "
                  "registered",
                  "The Uuid '", uuid, "' has not been registered."),

        _uuid(uuid) {}
  ~TypeNotRegistered() noexcept override = default;
  TypeNotRegistered(const TypeNotRegistered &) = default;
  TypeNotRegistered &operator=(const TypeNotRegistered &) = default;

  ::Smp::Uuid GetUuid() const noexcept override { return _uuid; }

private:
  ::Smp::Uuid _uuid;
};

class InvalidParameterIndex final : public Exception,
                                    public ::Smp::InvalidParameterIndex {
public:
  InvalidParameterIndex(const ::Smp::IObject *operation,
                        ::Smp::Int32 parameterIndex,
                        ::Smp::Int32 parameterCount)
      : Exception(
            operation, __func__,
            "This exception is raised when using an invalid parameter index to "
            "set (SetParameterValue()) or get (GetParameterValue()) a "
            "parameter value of an operation in a request",
            "The Operation '", operation, "' has '", parameterCount,
            "' parameter(s). Cannot access to parameter at index '",
            parameterIndex, "'."),

        _parameterIndex(parameterIndex), _parameterCount(parameterCount) {}
  ~InvalidParameterIndex() noexcept override = default;
  InvalidParameterIndex(const InvalidParameterIndex &) = default;
  InvalidParameterIndex &operator=(const InvalidParameterIndex &) = default;

  ::Smp::String8 GetOperationName() const noexcept override {
    return GetSender()->GetName();
  }
  ::Smp::Int32 GetParameterIndex() const noexcept override {
    return _parameterIndex;
  }
  ::Smp::Int32 GetParameterCount() const noexcept override {
    return _parameterCount;
  }

private:
  ::Smp::Int32 _parameterIndex;
  ::Smp::Int32 _parameterCount;
};

class VoidOperation final : public Exception, public ::Smp::VoidOperation {
public:
  explicit VoidOperation(const ::Smp::IObject *sender)
      : Exception(sender, __func__,
                  "Cannot read (GetReturnValue()) or write (SetReturnValue()) "
                  "the return value of a void operation",
                  "The Operation '", sender, "' does not have a return value") {
  }
  ~VoidOperation() noexcept override = default;
  VoidOperation(const VoidOperation &) = default;
  VoidOperation &operator=(const VoidOperation &) = default;

  ::Smp::String8 GetOperationName() const noexcept override {
    return GetSender()->GetName();
  }
};

class InvalidAnyType final : public Exception, public ::Smp::InvalidAnyType {
public:
  InvalidAnyType(const ::Smp::IObject *sender,
                 ::Smp::PrimitiveTypeKind expectedType,
                 ::Smp::PrimitiveTypeKind invalidType)
      : Exception(sender, __func__,
                  "Cannot use an AnySimple argument of wrong type",

                  "The AnySimple type '", invalidType,
                  "' is invalid: expected '", expectedType, "'."),

        _expectedType(expectedType), _invalidType(invalidType) {}
  ~InvalidAnyType() noexcept override = default;
  InvalidAnyType(const InvalidAnyType &) = default;
  InvalidAnyType &operator=(const InvalidAnyType &) = default;

  ::Smp::PrimitiveTypeKind GetInvalidType() const noexcept override {
    return _invalidType;
  }

  ::Smp::PrimitiveTypeKind GetExpectedType() const noexcept override {
    return _expectedType;
  }

private:
  ::Smp::PrimitiveTypeKind _expectedType;
  ::Smp::PrimitiveTypeKind _invalidType;
};

class InvalidReturnValue final : public Exception,
                                 public ::Smp::InvalidReturnValue {
public:
  InvalidReturnValue(const ::Smp::IObject *sender,
                     const ::Smp::AnySimple &value)
      : Exception(sender, __func__,
                  "Cannot assign an invalid return value of an operation in a "
                  "request using SetReturnValue()",
                  "The return value '", value, "' is invalid for Operation '",
                  sender, "'."),
        _value(value) {}
  ~InvalidReturnValue() noexcept override = default;
  InvalidReturnValue(const InvalidReturnValue &) = default;
  InvalidReturnValue &operator=(const InvalidReturnValue &) = default;

  ::Smp::String8 GetOperationName() const noexcept override {
    return GetSender()->GetName();
  }

  ::Smp::AnySimple GetValue() const noexcept override { return _value; }

private:
  ::Smp::AnySimple _value;
};

class InvalidParameterValue final : public Exception,
                                    public ::Smp::InvalidParameterValue {
public:
  InvalidParameterValue(const ::Smp::IObject *sender,
                        std::string_view parameterName,
                        const ::Smp::AnySimple &value)
      : Exception(sender, __func__,
                  "Cannot assign an illegal value to a parameter of an "
                  "operation in a request using SetParameterValue()",
                  "The value '", value, "' is invalid for parameter '",
                  parameterName, "' in Operation '", sender, "'."),

        _parameterName(parameterName), _value(value) {}
  ~InvalidParameterValue() noexcept override = default;
  InvalidParameterValue(const InvalidParameterValue &) = default;
  InvalidParameterValue &operator=(const InvalidParameterValue &) = default;

  ::Smp::String8 GetParameterName() const noexcept override {
    return _parameterName.c_str();
  }
  ::Smp::AnySimple GetValue() const noexcept override { return _value; }

private:
  std::string _parameterName;
  ::Smp::AnySimple _value;
};

class InvalidFieldType final : public Exception,
                               public ::Smp::InvalidFieldType {
public:
  explicit InvalidFieldType(const ::Smp::IObject *sender,
                            const ::Smp::Uuid &uuid)
      : Exception(sender, __func__, "Cannot publish a field with invalid type",
                  "The type UUID '", uuid, "' is invalid for field '", sender,
                  "'.") {}
  explicit InvalidFieldType(const ::Smp::IObject *sender,
                            ::Smp::PrimitiveTypeKind kind)
      : Exception(sender, __func__, "Cannot publish a field with invalid type",
                  "The primitive type kind '", kind, "' is invalid for field '",
                  sender, "'.") {}
  explicit InvalidFieldType(const ::Smp::IObject *sender,
                            const ::Smp::Publication::IType *type)
      : Exception(sender, __func__, "Cannot publish a field with invalid type",
                  "The type '", type->GetName(), "' is invalid for field '",
                  sender, "'.") {}
  ~InvalidFieldType() noexcept override = default;
  InvalidFieldType(const InvalidFieldType &) = default;
  InvalidFieldType &operator=(const InvalidFieldType &) = default;
};

class DuplicateUuid final : public Exception, public ::Smp::DuplicateUuid {
public:
  DuplicateUuid(const ::Smp::IObject *sender, const ::Smp::IFactory *oldFactory,
                ::Smp::String8 newName)
      : Exception(sender, __func__,
                  "Cannot register a factory under a Uuid that has already "
                  "been used to register another (or the same) factory. This "
                  "would lead to duplicate implementation Uuids",
                  "The Uuid '", oldFactory->GetUuid(), "' of Factory '",
                  nullCheck(newName), "' collides with the existing factory '",
                  oldFactory->GetName(), "'."),

        _oldFactory(oldFactory), _newName(nullCheck(newName)) {}
  ~DuplicateUuid() noexcept override = default;
  DuplicateUuid(const DuplicateUuid &) = default;
  DuplicateUuid &operator=(const DuplicateUuid &) = default;

  ::Smp::String8 GetOldName() const noexcept override {
    return _oldFactory->GetName();
  }

  ::Smp::String8 GetNewName() const noexcept override {
    return _newName.c_str();
  }

private:
  const ::Smp::IFactory *_oldFactory;
  std::string _newName;
};

class LibraryNotFound final : public Exception, public ::Smp::LibraryNotFound {
public:
  LibraryNotFound(const ::Smp::IObject *sender, ::Smp::String8 libraryName,
                  std::string_view error)
      : Exception(sender, __func__, "Cannot load a library that does not exist",
                  error),

        _libraryName(nullCheck(libraryName)) {}
  ~LibraryNotFound() noexcept override = default;
  LibraryNotFound(const LibraryNotFound &) = default;
  LibraryNotFound &operator=(const LibraryNotFound &) = default;

  ::Smp::String8 GetLibraryName() const noexcept override {
    return _libraryName.c_str();
  }

private:
  std::string _libraryName;
};

class InvalidLibrary final : public Exception, public ::Smp::InvalidLibrary {
public:
  InvalidLibrary(const ::Smp::IObject *sender, ::Smp::String8 libraryName,
                 std::string_view msg)
      : Exception(sender, __func__,
                  "Cannot load an undefined symbol from a library", msg),
        _libraryName(nullCheck(libraryName)) {}
  ~InvalidLibrary() noexcept override = default;
  InvalidLibrary(const InvalidLibrary &) = default;
  InvalidLibrary &operator=(const InvalidLibrary &) = default;

  ::Smp::String8 GetLibraryName() const noexcept override {
    return _libraryName.c_str();
  }

private:
  std::string _libraryName;
};

class InvalidSimulationTime final
    : public Exception,
      public ::Smp::Services::InvalidSimulationTime {
public:
  InvalidSimulationTime(const ::Smp::IObject *sender, ::Smp::Duration current,
                        ::Smp::Duration provided, ::Smp::Duration max)
      : Exception(sender, __func__,
                  "This exception is thrown by SetSimulationTime if the new "
                  "simulation time is not between the current simulation time "
                  "and the simulation time of the next event on the scheduler",
                  "The simulation time '", provided,
                  "' is invalid. Expecting a value between '", current,
                  "' and '", max, "'."),

        _current(current), _provided(provided), _max(max) {}
  ~InvalidSimulationTime() noexcept override = default;
  InvalidSimulationTime(const InvalidSimulationTime &) = default;
  InvalidSimulationTime &operator=(const InvalidSimulationTime &) = default;

  ::Smp::Duration GetCurrentTime() const noexcept override { return _current; }

  ::Smp::Duration GetProvidedTime() const noexcept override {
    return _provided;
  }

  ::Smp::Duration GetMaximumTime() const noexcept override { return _max; }

private:
  ::Smp::Duration _current;
  ::Smp::Duration _provided;
  ::Smp::Duration _max;
};

class TypeAlreadyRegistered final
    : public Exception,
      public ::Smp::Publication::TypeAlreadyRegistered {
public:
  TypeAlreadyRegistered(const ::Smp::IObject *sender, ::Smp::String8 typeName,
                        const ::Smp::Publication::IType *type)
      : Exception(sender, __func__,
                  "Cannot register a type with a Uuid that has already been "
                  "registered",
                  "Cannot register '", nullCheck(typeName), "' with Uuid '",
                  type->GetUuid(), "'. This Uuid has already been registered."),

        _typeName(nullCheck(typeName)), _type(type) {}
  ~TypeAlreadyRegistered() noexcept override = default;
  TypeAlreadyRegistered(const TypeAlreadyRegistered &) = default;
  TypeAlreadyRegistered &operator=(const TypeAlreadyRegistered &) = default;

  ::Smp::String8 GetTypeName() const noexcept override {
    return _typeName.c_str();
  }

  const ::Smp::Publication::IType *GetType() const noexcept override {
    return _type;
  }

private:
  std::string _typeName;
  const ::Smp::Publication::IType *_type;
};

class DuplicateLiteral final : public Exception,
                               public ::Smp::Publication::DuplicateLiteral {
public:
  DuplicateLiteral(const ::Smp::IObject *sender, ::Smp::String8 literalName,
                   ::Smp::Int32 literalValue)
      : Exception(sender, __func__,
                  "Cannot add a literal to an enumeration using a value that "
                  "has been used for another literal before.",
                  "Value '", literalValue, "' is already used by '",
                  nullCheck(literalName), "' literal."),

        _literalName(nullCheck(literalName)), _literalValue(literalValue) {}
  ~DuplicateLiteral() noexcept override = default;
  DuplicateLiteral(const DuplicateLiteral &) = default;
  DuplicateLiteral &operator=(const DuplicateLiteral &) = default;

  ::Smp::String8 GetLiteralName() const noexcept override {
    return _literalName.c_str();
  }

  /// Get the value of the literal that has been used before.
  /// @return  Value of the literal.
  ::Smp::Int32 GetLiteralValue() const noexcept override {
    return _literalValue;
  }

private:
  std::string _literalName;
  ::Smp::Int32 _literalValue;
};

class InvalidPrimitiveType final
    : public Exception,
      public ::Smp::Publication::InvalidPrimitiveType {
public:
  InvalidPrimitiveType(const ::Smp::IObject *sender, ::Smp::String8 typeName,
                       ::Smp::PrimitiveTypeKind type)
      : Exception(sender, __func__,
                  "Cannot use an invalid primitive type kind as parameter for "
                  "a user-defined float or integer type",

                  "The primitive type '", type, "' is invalid for '",
                  nullCheck(typeName), "'."),

        _typeName(nullCheck(typeName)), _type(type) {}
  ~InvalidPrimitiveType() noexcept override = default;
  InvalidPrimitiveType(const InvalidPrimitiveType &) = default;
  InvalidPrimitiveType &operator=(const InvalidPrimitiveType &) = default;

  ::Smp::String8 GetTypeName() const noexcept override {
    return _typeName.c_str();
  }

  ::Smp::PrimitiveTypeKind GetType() const noexcept override { return _type; }

private:
  std::string _typeName;
  ::Smp::PrimitiveTypeKind _type;
};

class InvalidSimulatorState final : public Exception,
                                    public ::Smp::InvalidSimulatorState {
public:
  InvalidSimulatorState(const ::Smp::IObject *sender,
                        ::Smp::SimulatorStateKind state)
      : Exception(sender, __func__,
                  "This exception is raised by the simulator when one of the "
                  "operations is called in an invalid state",
                  "Cannot performs an operation while the simulator is in '",
                  state, "' state."),
        _state(state) {}
  ~InvalidSimulatorState() noexcept override = default;
  InvalidSimulatorState(const InvalidSimulatorState &) = default;
  InvalidSimulatorState &operator=(const InvalidSimulatorState &) = default;

  ::Smp::SimulatorStateKind GetInvalidState() const noexcept override {
    return _state;
  }

private:
  ::Smp::SimulatorStateKind _state;
};

namespace detail {
std::ostream &operator<<(std::ostream &ostream, const ::Smp::IObject *obj) {
  return ostream << ::Xsmp::Helper::GetPath(obj);
}
void throwException(const ::Smp::IObject *sender, std::string_view name,
                    std::string_view description, std::string_view message) {
  throw Exception(sender, name, description, message);
}

} // namespace detail

void throwFieldAlreadyConnected(const ::Smp::IObject *sender,
                                const ::Smp::IDataflowField *source,
                                const ::Smp::IField *target) {
  throw FieldAlreadyConnected(sender, source, target);
}

void throwInvalidTarget(const ::Smp::IObject *sender,
                        const ::Smp::IDataflowField *source,
                        const ::Smp::IField *target) {
  throw InvalidTarget(sender, source, target);
}

void throwCannotStore(const ::Smp::IObject *sender, std::string_view msg) {
  throw CannotStore(sender, msg);
}

void throwCannotRestore(const ::Smp::IObject *sender, std::string_view msg) {
  throw CannotRestore(sender, msg);
}

void throwInvalidObjectName(const ::Smp::IObject *sender, ::Smp::String8 name,
                            std::string_view description) {
  throw InvalidObjectName(sender, name, description);
}

void throwContainerFull(const ::Smp::IContainer *sender) {
  throw ContainerFull(sender);
}

void throwDuplicateName(const ::Smp::IObject *sender,
                        ::Smp::String8 duplicateName,
                        const ::Smp::IObject *collection) {
  throw DuplicateName(sender, duplicateName, collection);
}

void throwNotContained(const ::Smp::IContainer *container,
                       ::Smp::IComponent *component) {
  throw NotContained(container, component);
}

void throwCannotDelete(const ::Smp::IContainer *container,
                       ::Smp::IComponent *component) {
  throw CannotDelete(container, component);
}

void throwInvalidComponentState(const ::Smp::IComponent *sender,
                                ::Smp::ComponentStateKind invalidState,
                                ::Smp::ComponentStateKind expectedState) {
  throw InvalidComponentState(sender, invalidState, expectedState);
}

void throwInvalidObjectType(const ::Smp::IObject *sender,
                            ::Smp::IObject *invalidObject,
                            const std::type_info &expectedType) {
  throw InvalidObjectType(sender, invalidObject, expectedType);
}

void throwNotReferenced(const ::Smp::IReference *sender,
                        ::Smp::IComponent *component) {
  throw NotReferenced(sender, component);
}

void throwReferenceFull(const ::Smp::IReference *sender,
                        ::Smp::Int64 referenceSize) {
  throw ReferenceFull(sender, referenceSize);
}

void throwCannotRemove(const ::Smp::IReference *sender,
                       ::Smp::IComponent *component, ::Smp::Int64 lowerLimit) {
  throw CannotRemove(sender, component, lowerLimit);
}

void throwInvalidEventSink(const ::Smp::IObject *sender,
                           const ::Smp::IEventSource *eventSource,
                           const ::Smp::IEventSink *eventSink) {
  throw InvalidEventSink(sender, eventSource, eventSink);
}

void throwEventSinkAlreadySubscribed(const ::Smp::IObject *sender,
                                     const ::Smp::IEventSource *eventSource,
                                     const ::Smp::IEventSink *eventSink) {
  throw EventSinkAlreadySubscribed(sender, eventSource, eventSink);
}

void throwEventSinkNotSubscribed(const ::Smp::IObject *sender,
                                 const ::Smp::IEventSource *eventSource,
                                 const ::Smp::IEventSink *eventSink) {
  throw EventSinkNotSubscribed(sender, eventSource, eventSink);
}

void throwInvalidOperationName(const ::Smp::IObject *sender,
                               ::Smp::String8 operationName) {
  throw InvalidOperationName(sender, operationName);
}

void throwInvalidParameterCount(const ::Smp::IOperation *sender,
                                ::Smp::Int32 requestedNbParameters) {
  throw InvalidParameterCount(sender, requestedNbParameters);
}

void throwInvalidParameterType(const ::Smp::IObject *sender,
                               std::string_view operationName,
                               std::string_view parameterName,
                               ::Smp::PrimitiveTypeKind invalidType,
                               ::Smp::PrimitiveTypeKind expectedType) {
  throw InvalidParameterType(sender, operationName, parameterName, invalidType,
                             expectedType);
}

void throwInvalidArrayIndex(const ::Smp::IArrayField *sender,
                            ::Smp::UInt64 invalidIndex) {
  throw InvalidArrayIndex(sender, static_cast<::Smp::Int64>(sender->GetSize()),
                          static_cast<::Smp::Int64>(invalidIndex));
}
void throwInvalidArrayIndex(const ::Smp::ISimpleArrayField *sender,
                            ::Smp::UInt64 invalidIndex) {
  throw InvalidArrayIndex(sender, static_cast<::Smp::Int64>(sender->GetSize()),
                          static_cast<::Smp::Int64>(invalidIndex));
}

void throwInvalidFieldValue(const ::Smp::IField *sender,
                            const ::Smp::AnySimple &invalidFieldValue) {
  throw InvalidFieldValue(sender, invalidFieldValue);
}

void throwInvalidArrayValue(const ::Smp::ISimpleArrayField *sender,
                            ::Smp::UInt64 index,
                            const ::Smp::AnySimple &invalidValue) {
  throw InvalidArrayValue(sender, static_cast<::Smp::Int64>(index),
                          invalidValue);
}

void throwInvalidArraySize(const ::Smp::ISimpleArrayField *sender,
                           ::Smp::UInt64 invalidSize) {
  throw InvalidArraySize(sender, static_cast<::Smp::Int64>(invalidSize));
}

void throwInvalidEventName(const ::Smp::IObject *sender,
                           ::Smp::String8 eventName) {
  throw InvalidEventName(sender, eventName);
}

void throwInvalidEventId(const ::Smp::IObject *sender,
                         ::Smp::Services::EventId eventId) {
  throw InvalidEventId(sender, eventId);
}

void throwInvalidCycleTime(const ::Smp::IObject *sender,
                           ::Smp::Duration cycleTime) {
  throw InvalidCycleTime(sender, Duration{cycleTime});
}

void throwInvalidEventTime(const ::Smp::IObject *sender,
                           ::Smp::Duration eventTime,
                           ::Smp::Duration currentTime) {
  throw InvalidEventTime(sender, Duration{eventTime}, Duration{currentTime});
}

void throwEntryPointNotSubscribed(const ::Smp::IObject *sender,
                                  const ::Smp::IEntryPoint *entryPoint,
                                  std::string_view eventName) {
  throw EntryPointNotSubscribed(sender, entryPoint, eventName);
}

void throwEntryPointAlreadySubscribed(const ::Smp::IObject *sender,
                                      const ::Smp::IEntryPoint *entryPoint,
                                      std::string_view eventName) {
  throw EntryPointAlreadySubscribed(sender, entryPoint, eventName);
}

void throwInvalidFieldName(const ::Smp::IObject *sender,
                           ::Smp::String8 invalidFieldName) {
  throw InvalidFieldName(sender, invalidFieldName);
}

void throwTypeNotRegistered(const ::Smp::IObject *sender, ::Smp::Uuid uuid) {
  throw TypeNotRegistered(sender, uuid);
}

void throwInvalidParameterIndex(const ::Smp::IObject *sender,
                                ::Smp::Int32 index, ::Smp::Int32 count) {
  throw InvalidParameterIndex(sender, index, count);
}

void throwVoidOperation(const ::Smp::IObject *sender) {
  throw VoidOperation(sender);
}

void throwInvalidAnyType(const ::Smp::IObject *sender,
                         ::Smp::PrimitiveTypeKind expectedType,
                         ::Smp::PrimitiveTypeKind invalidType) {
  throw InvalidAnyType(sender, expectedType, invalidType);
}

void throwInvalidReturnValue(const ::Smp::IObject *sender,
                             const ::Smp::AnySimple &value) {
  throw InvalidReturnValue(sender, value);
}

void throwInvalidParameterValue(const ::Smp::IObject *sender,
                                std::string_view parameterName,
                                const ::Smp::AnySimple &value) {
  throw InvalidParameterValue(sender, parameterName, value);
}

void throwInvalidFieldType(const ::Smp::IObject *sender,
                           const ::Smp::Uuid &uuid) {
  throw InvalidFieldType(sender, uuid);
}
void throwInvalidFieldType(const ::Smp::IObject *sender,
                           const ::Smp::Publication::IType *type) {
  throw InvalidFieldType(sender, type);
}
void throwInvalidFieldType(const ::Smp::IObject *sender,
                           ::Smp::PrimitiveTypeKind kind) {
  throw InvalidFieldType(sender, kind);
}
void throwDuplicateUuid(const ::Smp::IObject *sender,
                        const ::Smp::IFactory *oldFactory,
                        ::Smp::String8 newName) {
  throw DuplicateUuid(sender, oldFactory, newName);
}

void throwLibraryNotFound(const ::Smp::IObject *sender,
                          ::Smp::String8 libraryName, std::string_view msg) {
  throw LibraryNotFound(sender, libraryName, msg);
}

void throwInvalidLibrary(const ::Smp::IObject *sender,
                         ::Smp::String8 libraryName, std::string_view msg) {
  throw InvalidLibrary(sender, libraryName, msg);
}

void throwInvalidSimulationTime(const ::Smp::IObject *sender,
                                ::Smp::Duration current,
                                ::Smp::Duration provided, ::Smp::Duration max) {
  throw InvalidSimulationTime(sender, current, provided, max);
}
void throwTypeAlreadyRegistered(const ::Smp::IObject *sender,
                                ::Smp::String8 newTypeName,
                                const ::Smp::Publication::IType *existingType) {
  throw TypeAlreadyRegistered(sender, newTypeName, existingType);
}

void throwInvalidPrimitiveType(const ::Smp::IObject *sender,
                               ::Smp::String8 typeName,
                               ::Smp::PrimitiveTypeKind kind) {
  throw InvalidPrimitiveType(sender, typeName, kind);
}

void throwInvalidSimulatorState(const ::Smp::IObject *sender,
                                ::Smp::SimulatorStateKind state) {
  throw InvalidSimulatorState(sender, state);
}

void throwDuplicateLiteral(const ::Smp::IObject *sender,
                           ::Smp::String8 literalName,
                           ::Smp::Int32 literalValue) {
  throw DuplicateLiteral(sender, literalName, literalValue);
}
} // namespace Xsmp::Exception
