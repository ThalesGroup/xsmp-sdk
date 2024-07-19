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

#ifndef XSMP_EVENTPROVIDER_H_
#define XSMP_EVENTPROVIDER_H_

#include <Smp/IEventProvider.h>
#include <Smp/IEventSource.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Collection.h>

/// XSMP standard types and interfaces.
namespace Xsmp {

/// XSMP implementation details.
namespace detail {
class AbstractEventSource;
} // namespace detail

/// @class EventProvider
/// XSMP implementation of ::Smp::IEventProvider.
class EventProvider : public virtual ::Smp::IEventProvider {
public:
  /// Default constructor
  EventProvider();

  /// Query for the collection of all event sources of the component.
  /// The collection may be empty if no event sources exist.
  /// @return  Collection of event sources.
  const ::Smp::EventSourceCollection *GetEventSources() const override;

  /// Query for an event source of this component by its name.
  /// The returned event source may be null if no event source with the
  /// given name could be found.
  /// @param   name Event source name.
  /// @return  Event source with the given name or null if no event
  ///          source with the given name could be found.
  ::Smp::IEventSource *GetEventSource(::Smp::String8 name) const override;

protected:
  /// Adds an event source to this EventProvider.
  /// @param eventSource Pointer to the event source to add
  inline void AddEventSource(::Smp::IEventSource *eventSource) {
    _event_sources.Add(eventSource);
  }

private:
  friend ::Xsmp::detail::AbstractEventSource;
  ::Xsmp::Collection<::Smp::IEventSource> _event_sources;
};

} // namespace Xsmp
#endif // XSMP_EVENTPROVIDER_H_
