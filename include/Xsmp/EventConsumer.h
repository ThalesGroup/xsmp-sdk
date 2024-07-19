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

#ifndef XSMP_EVENTCONSUMER_
#define XSMP_EVENTCONSUMER_

#include <Smp/IEventConsumer.h>
#include <Smp/IEventSink.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Collection.h>

/// XSMP standard types and interfaces.
namespace Xsmp {

/// XSMP implementation details.
namespace detail {
class AbstractEventSink;
} // namespace detail

/// @class EventConsumer
/// XSMP implementation of ::Smp::IEventConsumer.
class EventConsumer : public virtual ::Smp::IEventConsumer {
public:
  /// Default constructor
  EventConsumer();

  /// Query for the collection of all event sinks of the component.
  /// The collection may be empty if no event sinks exist.
  /// @return  Collection of event sinks.
  const ::Smp::EventSinkCollection *GetEventSinks() const override;

  /// Query for an event sink of this component by its name.
  /// The returned event sink may be null if no event sink with the given
  /// name could be found.
  /// @param   name Event sink name.
  /// @return  Event sink with the given name, or null if no event sink
  ///          with the given name could be found.
  ::Smp::IEventSink *GetEventSink(::Smp::String8 name) const override;

protected:
  /// Adds an event sink to this EventConsumer.
  /// @param eventSink Pointer to the event sink to add
  inline void AddEventSink(::Smp::IEventSink *eventSink) {
    _event_sinks.Add(eventSink);
  }

private:
  friend ::Xsmp::detail::AbstractEventSink;
  ::Xsmp::Collection<::Smp::IEventSink> _event_sinks;
};

} // namespace Xsmp
#endif // XSMP_EVENTCONSUMER_
