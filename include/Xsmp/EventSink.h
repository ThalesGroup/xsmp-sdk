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

#ifndef XSMP_EVENTSINK_H_
#define XSMP_EVENTSINK_H_

#include <Smp/AnySimple.h>
#include <Smp/IEventSink.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/AnySimpleConverter.h>
#include <Xsmp/cstring.h>
#include <functional>

/// XSMP standard types and interfaces.
namespace Xsmp {

class EventConsumer;

/// XSMP implementation details.
namespace detail {
class AbstractEventSink : public ::Smp::IEventSink {
public:
  AbstractEventSink(::Smp::String8 name, ::Smp::String8 description,
                    ::Xsmp::EventConsumer *parent);
  AbstractEventSink(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::IObject *parent);
  AbstractEventSink(const AbstractEventSink &) = delete;
  AbstractEventSink &operator=(const AbstractEventSink &) = delete;
  ~AbstractEventSink() noexcept override = default;
  ::Smp::String8 GetName() const final;
  ::Smp::String8 GetDescription() const final;
  ::Smp::IObject *GetParent() const final;

private:
  ::Xsmp::cstring _name;
  ::Xsmp::cstring _description;
  ::Smp::IObject *_parent;
};
} // namespace detail

/// @class EventSink
/// XSMP implementation of ::Smp::IEventSink with argument.
/// @tparam T The type of argument that this EvetnSink instance will handle.
template <typename T = void>
class EventSink final : public detail::AbstractEventSink {
  using callbackType = std::function<void(::Smp::IObject *sender, T value)>;

public:
  /// Constructs a new event sink object with the specified name,
  /// description, parent, callback and event argument type.
  /// @param name The name of the event sink.
  /// @param description The description of the event sink.
  /// @param parent The parent object of the event sink.
  /// @param callback A function object representing the action to be executed
  /// when this event sink is invoked.
  /// @param eventArgType The event argument type.
  EventSink(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, callbackType &&callback,
            ::Smp::PrimitiveTypeKind eventArgType)
      : AbstractEventSink(name, description, parent),
        _callback(std::move(callback)), _eventArgType(eventArgType) {}

  /// Constructs a new event sink object with the specified name,
  /// description, parent, callback and event argument type.
  /// @param name The name of the event sink.
  /// @param description The description of the event sink.
  /// @param parent The parent object of the event sink.
  /// @param callback A function object representing the action to be executed
  /// when this event sink is invoked.
  /// @param eventArgType The event argument type.
  EventSink(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::EventConsumer *parent, callbackType &&callback,
            ::Smp::PrimitiveTypeKind eventArgType)
      : AbstractEventSink(name, description, parent),
        _callback(std::move(callback)), _eventArgType(eventArgType) {}

  /// Get the primitive type kind of the event argument.
  /// This operation allows for type checking between an Event Source
  /// (implementing IEventSource) and an event sink (implementing
  /// IEventSink) during Subscribe.
  /// @return  Primitive type kind of the event argument.
  ::Smp::PrimitiveTypeKind GetEventArgType() const override {
    return _eventArgType;
  }

  /// This event handler method is called when an event is emitted.
  /// Components providing event sinks must ensure that these event sinks
  /// do not throw exceptions.
  /// @param sender Object emitting the event.
  /// @param value Event argument with context data for event notification.
  /// The type of the event argument depends on the type of the event sink.
  void Notify(::Smp::IObject *sender, ::Smp::AnySimple value) override {
    std::invoke(_callback, sender, AnySimpleConverter<T>::convert(value));
  }

private:
  callbackType _callback;
  ::Smp::PrimitiveTypeKind _eventArgType;
};

/// @class EventSink
/// XSMP implementation of ::Smp::IEventSink without argument.
template <> class EventSink<void> final : public detail::AbstractEventSink {
  using callbackType = std::function<void(::Smp::IObject *sender)>;

public:
  /// Constructs a new event sink object with the specified name,
  /// description, parent and callback.
  /// @param name The name of the event sink.
  /// @param description The description of the event sink.
  /// @param parent The parent object of the event sink.
  /// @param callback A function object representing the action to be executed
  /// when this event sink is invoked.
  EventSink(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, callbackType &&callback);

  /// Constructs a new event sink object with the specified name,
  /// description, parent and callback.
  /// @param name The name of the event sink.
  /// @param description The description of the event sink.
  /// @param parent The parent object of the event sink.
  /// @param callback A function object representing the action to be executed
  /// when this event sink is invoked.
  EventSink(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::EventConsumer *parent, callbackType &&callback);

  /// Get the None primitive type kind.
  /// @return  PTK_None.
  ::Smp::PrimitiveTypeKind GetEventArgType() const override;

  /// This event handler method is called when an event is emitted.
  /// Components providing event sinks must ensure that these event sinks
  /// do not throw exceptions.
  /// @param sender Object emitting the event.
  /// @param value Unused event argument.
  void Notify(::Smp::IObject *sender, ::Smp::AnySimple value) override;

private:
  callbackType _callback;
};

// deduction guide
EventSink(
    ::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
    std::function<void(::Smp::IObject *sender)> callback) -> EventSink<void>;

} // namespace Xsmp

#endif // XSMP_EVENTSINK_H_
