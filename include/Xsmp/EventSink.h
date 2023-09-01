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
#include <Xsmp/Object.h>

#include <functional>

namespace Xsmp {

class EventConsumer;

namespace detail {
class AbstractEventSink: public Object, public ::Smp::IEventSink {
public:
    AbstractEventSink(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::EventConsumer *parent);
    AbstractEventSink(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent);
};
} // namespace detail

template<typename T = void>
class EventSink final: public detail::AbstractEventSink {
public:

    template<typename ObjectType>
    EventSink(::Smp::String8 name, ::Smp::String8 description,
            ObjectType *parent,
            void (ObjectType::*callback)(::Smp::IObject*, T),
            ::Smp::PrimitiveTypeKind eventArgType) :
            EventSink(name, description, parent,
                    std::bind(callback, parent, std::placeholders::_1,
                            std::placeholders::_2), eventArgType) {
    }

    template<typename Callable>
    EventSink(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, Callable &&callback,
            ::Smp::PrimitiveTypeKind eventArgType) :
            AbstractEventSink(name, description, parent), _callback(
                    std::forward<Callable>(callback)), _eventArgType(
                    eventArgType) {
    }

    template<typename Callable>
    EventSink(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::EventConsumer *parent, Callable &&callback,
            ::Smp::PrimitiveTypeKind eventArgType) :
            AbstractEventSink(name, description, parent), _callback(
                    std::forward<Callable>(callback)), _eventArgType(
                    eventArgType) {
    }

    ::Smp::PrimitiveTypeKind GetEventArgType() const override {
        return _eventArgType;
    }
    void Notify(::Smp::IObject *sender, ::Smp::AnySimple value) override {
        std::invoke(_callback, sender, AnySimpleConverter<T>::convert(value));
    }

private:
    std::function<void(::Smp::IObject *sender, T value)> _callback;
    ::Smp::PrimitiveTypeKind _eventArgType;
};

/// specialization for void event (implemented in .cpp)
template<>
class EventSink<void> final: public detail::AbstractEventSink {
public:

    template<typename ObjectType>
    EventSink(::Smp::String8 name, ::Smp::String8 description,
            ObjectType *parent, void (ObjectType::*callback)(::Smp::IObject*)) :
            EventSink(name, description, parent,
                    std::bind(callback, parent, std::placeholders::_1)) {
    }

    template<typename Callable>
    EventSink(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, Callable &&callback) :
            AbstractEventSink(name, description, parent), _callback(
                    std::forward<Callable>(callback)) {
    }

    template<typename Callable>
    EventSink(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::EventConsumer *parent, Callable &&callback) :
            AbstractEventSink(name, description, parent), _callback(
                    std::forward<Callable>(callback)) {
    }
    ~EventSink() noexcept override = default;
    ::Smp::PrimitiveTypeKind GetEventArgType() const override;
    void Notify(::Smp::IObject *sender, ::Smp::AnySimple value) override;

private:
    std::function<void(::Smp::IObject *sender)> _callback;
};

} // namespace Xsmp

#endif // XSMP_EVENTSINK_H_
