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

namespace Xsmp {

namespace detail {
class AbstractEventSink;
} // namespace detail

class EventConsumer : public virtual ::Smp::IEventConsumer {
public:
  EventConsumer();
  const ::Smp::EventSinkCollection *GetEventSinks() const override;
  ::Smp::IEventSink *GetEventSink(::Smp::String8 name) const override;

protected:
  inline void AddEventSink(::Smp::IEventSink *eventSink) {
    _event_sinks.Add(eventSink);
  }

private:
  friend ::Xsmp::detail::AbstractEventSink;
  ::Xsmp::Collection<::Smp::IEventSink> _event_sinks;
};

} // namespace Xsmp
#endif // XSMP_EVENTCONSUMER_
