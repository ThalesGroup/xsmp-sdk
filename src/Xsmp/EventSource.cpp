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

#include <Xsmp/EventProvider.h>
#include <Xsmp/EventSource.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <algorithm>
#include <iterator>

namespace Xsmp {
namespace detail {

AbstractEventSource::AbstractEventSource(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Xsmp::EventProvider *parent,
                                         ::Smp::PrimitiveTypeKind eventArgType)
    : AbstractEventSource(name, description,
                          static_cast<::Smp::IObject *>(parent), eventArgType) {
  parent->AddEventSource(this);
}
AbstractEventSource::AbstractEventSource(::Smp::String8 name,
                                         ::Smp::String8 description,
                                         ::Smp::IObject *parent,
                                         ::Smp::PrimitiveTypeKind eventArgType)
    : _name(::Xsmp::Helper::checkName(name, parent)),
      _description(description ? description : ""), _parent(parent),
      _eventArgType{eventArgType} {}
::Smp::String8 AbstractEventSource::GetName() const { return _name.c_str(); }

::Smp::String8 AbstractEventSource::GetDescription() const {
  return _description.c_str();
}

::Smp::IObject *AbstractEventSource::GetParent() const { return _parent; }
void AbstractEventSource::Subscribe(::Smp::IEventSink *eventSink) {
  // Check if the type matches
  if (eventSink->GetEventArgType() != _eventArgType) {
    ::Xsmp::Exception::throwInvalidEventSink(this, this, eventSink);
  }

  if (std::find(_event_sinks.begin(), _event_sinks.end(), eventSink) !=
      _event_sinks.end())
    ::Xsmp::Exception::throwEventSinkAlreadySubscribed(this, this, eventSink);

  _event_sinks.insert(eventSink);
}
void AbstractEventSource::Unsubscribe(::Smp::IEventSink *eventSink) {
  auto it = std::find(_event_sinks.begin(), _event_sinks.end(), eventSink);
  if (it == _event_sinks.end())
    ::Xsmp::Exception::throwEventSinkNotSubscribed(this, this, eventSink);

  _event_sinks.erase(it);
}

void AbstractEventSource::RemoveLinks(
    const ::Smp::IComponent *target) noexcept {
  // remove all event sinks contained in parent
  for (auto it = _event_sinks.begin(); it != _event_sinks.end();) {
    if ((*it)->GetParent() == target)
      it = _event_sinks.erase(it);
    else
      ++it;
  }
}
void AbstractEventSource::DoEmit(::Smp::IObject *sender,
                                 const ::Smp::AnySimple &value) const {
  for (auto &sink : _event_sinks) {
    sink->Notify(sender, value);
  }
}

} // namespace detail

EventSource<void>::EventSource(::Smp::String8 name, ::Smp::String8 description,
                               ::Xsmp::EventProvider *parent)
    : AbstractEventSource(name, description, parent,
                          Smp::PrimitiveTypeKind::PTK_None) {}
EventSource<void>::EventSource(::Smp::String8 name, ::Smp::String8 description,
                               ::Smp::IObject *parent)
    : AbstractEventSource(name, description, parent,
                          Smp::PrimitiveTypeKind::PTK_None) {}

void EventSource<void>::Emit(::Smp::IObject *sender) const {
  AbstractEventSource::DoEmit(sender, ::Smp::AnySimple{});
}
void EventSource<void>::Emit() const { Emit(this->GetParent()); }
void EventSource<void>::operator()(::Smp::IObject *sender) const {
  Emit(sender);
}

void EventSource<void>::operator()() const { Emit(); }

} // namespace Xsmp
