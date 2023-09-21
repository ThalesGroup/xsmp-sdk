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

#ifndef XSMP_EVENTSOURCE_H_
#define XSMP_EVENTSOURCE_H_

#include <Smp/AnySimple.h>
#include <Smp/IEventSink.h>
#include <Smp/IEventSource.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/AnySimpleConverter.h>
#include <Xsmp/Object.h>
#include <set>

namespace Smp {
class IComponent;
} // namespace Smp

namespace Xsmp {
class Component;
class EventProvider;
namespace detail {

class AbstractEventSource: public Object, public ::Smp::IEventSource {
public:
    AbstractEventSource(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::EventProvider *parent,
            ::Smp::PrimitiveTypeKind eventArgType);
    AbstractEventSource(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, ::Smp::PrimitiveTypeKind eventArgType);

    void Subscribe(::Smp::IEventSink *eventSink) final;
    void Unsubscribe(::Smp::IEventSink *eventSink) final;
protected:
    inline ::Smp::PrimitiveTypeKind GetEventArgType() const noexcept {
        return _eventArgType;
    }

    void Emit(::Smp::IObject *sender, const ::Smp::AnySimple &value) const;
private:
    std::set<::Smp::IEventSink*> _event_sinks { };
    ::Smp::PrimitiveTypeKind _eventArgType;
    // Extensions for ILinkingComponent
    /// Asks an Event Source to remove all its links to the given target
    /// component.
    /// After this method has been called, the component must not try to
    /// access the given target component anymore.
    /// @param   target Target component to which all links shall be removed.
    virtual void RemoveLinks(const ::Smp::IComponent *target);
    friend ::Xsmp::Component;
};

} // namespace detail

template<typename T = void>
class EventSource final: public ::Xsmp::detail::AbstractEventSource {
public:
    EventSource(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::EventProvider *parent,
            ::Smp::PrimitiveTypeKind eventArgType) :
            AbstractEventSource(name, description, parent, eventArgType) {
    }
    EventSource(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, ::Smp::PrimitiveTypeKind eventArgType) :
            AbstractEventSource(name, description, parent, eventArgType) {
    }

    /// Emit the event to all connected events sinks
    /// @param sender component that emits the event
    /// @param value event value to send
    void Emit(::Smp::IObject *sender, T value) const {
        AbstractEventSource::Emit(sender,
                AnySimpleConverter<T>::convert(GetEventArgType(), value));

    }

    /// Emit the event to all connected events sinks
    /// @remark use the parent as emitter
    /// @param value event value to send

    void Emit(T value) const {
        Emit(this->GetParent(), value);
    }

    /// Emit the event to all connected events sinks
    /// @param sender component that emits the event
    /// @param value event value to send
    void operator()(::Smp::IObject *sender, T value) const {
        Emit(sender, value);
    }

    /// Emit the event to all connected events sinks
    /// @remark use the parent as emitter
    /// @param value event value to send
    void operator()(T value) {
        Emit(value);
    }
};

/// specialization for void event (implemented in .cpp)
template<>
class EventSource<void> final: public ::Xsmp::detail::AbstractEventSource {
public:
    EventSource(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::EventProvider *parent);
    EventSource(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent);

    /// Emit the event to all connected events sinks
    /// @param sender component that emits the event
    void Emit(::Smp::IObject *sender) const;

    /// Emit the event to all connected events sinks
    /// @remark use the parent as emitter
    void Emit() const;

    /// Emit the event to all connected events sinks
    /// @param sender component that emits the event
    void operator()(::Smp::IObject *sender) const;

    /// Emit the event to all connected events sinks
    /// @remark use the parent as emitter
    void operator()() const;
};

// deduction guide
EventSource(::Smp::String8 name, ::Smp::String8 description,
        ::Smp::IObject *parent) -> EventSource<void>;

} // namespace Xsmp

#endif // XSMP_EVENTSOURCE_H_
