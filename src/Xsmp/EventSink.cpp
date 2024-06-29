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

#include <Smp/PrimitiveTypes.h>
#include <Xsmp/EventConsumer.h>
#include <Xsmp/EventSink.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <utility>

namespace Xsmp {
namespace detail {

AbstractEventSink::AbstractEventSink(::Smp::String8 name,
                                     ::Smp::String8 description,
                                     EventConsumer *parent)
    : AbstractEventSink(name, description,
                        static_cast<::Smp::IObject *>(parent)) {
  parent->AddEventSink(this);
}
AbstractEventSink::AbstractEventSink(::Smp::String8 name,
                                     ::Smp::String8 description,
                                     ::Smp::IObject *parent)
    : _name(::Xsmp::Helper::checkName(name, parent)), _description(description),
      _parent(parent) {}
::Smp::String8 AbstractEventSink::GetName() const { return _name; }

::Smp::String8 AbstractEventSink::GetDescription() const {
  return _description;
}

::Smp::IObject *AbstractEventSink::GetParent() const { return _parent; }
} // namespace detail

::Smp::PrimitiveTypeKind EventSink<void>::GetEventArgType() const {
  return ::Smp::PrimitiveTypeKind::PTK_None;
}

EventSink<void>::EventSink(::Smp::String8 name, ::Smp::String8 description,
                           ::Smp::IObject *parent, callbackType &&callback)
    : AbstractEventSink(name, description, parent),
      _callback(std::move(callback)) {}
EventSink<void>::EventSink(::Smp::String8 name, ::Smp::String8 description,
                           ::Xsmp::EventConsumer *parent,
                           callbackType &&callback)
    : AbstractEventSink(name, description, parent),
      _callback(std::move(callback)) {}

void EventSink<void>::Notify(::Smp::IObject *sender, ::Smp::AnySimple value) {
  if (value.GetType() != GetEventArgType()) {
    ::Xsmp::Exception::throwInvalidAnyType(nullptr, value.GetType(),
                                           GetEventArgType());
  }
  _callback(sender);
}

} // namespace Xsmp
