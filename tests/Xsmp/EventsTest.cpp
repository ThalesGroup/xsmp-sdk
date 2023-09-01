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

#include <gtest/gtest.h>
#include <Smp/AnySimple.h>
#include <Smp/EventSinkAlreadySubscribed.h>
#include <Smp/EventSinkNotSubscribed.h>
#include <Smp/InvalidAnyType.h>
#include <Smp/InvalidEventSink.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Component.h>
#include <Xsmp/EventConsumer.h>
#include <Xsmp/EventProvider.h>
#include <Xsmp/EventSink.h>
#include <Xsmp/EventSource.h>
#include <type_traits>

namespace Xsmp {

namespace {
class TestEventProvider: public Xsmp::Component,
        public virtual Xsmp::EventProvider {
public:
    using Xsmp::Component::Component;
};
class TestEventConsumer: public Xsmp::Component,
        public virtual Xsmp::EventConsumer {
public:
    using Xsmp::Component::Component;
};
}

TEST(EventsTest, auto_register) {

    Object sender { "sender", "", nullptr };
    TestEventProvider event_sources { "collection" };

    EXPECT_EQ(0, event_sources.GetEventSources()->size());
    EventSource<void> eso { "eso", "", &event_sources };
    EXPECT_EQ(1, event_sources.GetEventSources()->size());
    EXPECT_EQ(&eso, event_sources.GetEventSource("eso"));
    EXPECT_EQ(&eso, event_sources.GetEventSources()->at("eso"));

    TestEventConsumer event_sinks { "collection", "", &sender };
    EXPECT_EQ(0, event_sinks.GetEventSinks()->size());
    EventSink<void> esi { "esi", "desc", &event_sinks, [&](::Smp::IObject*) {
    } };

    EXPECT_EQ(1, event_sinks.GetEventSinks()->size());
    EXPECT_EQ(&esi, event_sinks.GetEventSink("esi"));
    EXPECT_EQ(&esi, event_sinks.GetEventSinks()->at("esi"));
}

TEST(EventsTest, void_emit) {

    Object sender { "sender", "", nullptr };
    TestEventProvider event_sources { "collection", "", &sender };

    EventSource<void> eso { "eso1", "", &event_sources };

    eso.Emit(&sender);

    ::Smp::IObject *esiSender = nullptr;
    TestEventConsumer event_sinks { "collection", "", &sender };
    EventSink<void> esi { "esi1", "desc", &event_sinks, [&](
            ::Smp::IObject *obj) {
        esiSender = obj;
    } };

    eso.Subscribe(&esi);

    eso(&sender);
    EXPECT_EQ(&sender, esiSender);
    eso();
    EXPECT_EQ(&event_sources, esiSender);

    eso.Emit(nullptr);
    EXPECT_EQ(nullptr, esiSender);

    eso.Unsubscribe(&esi);
    eso.Emit(&sender);
    EXPECT_EQ(nullptr, esiSender);

}

TEST(EventsTest, bool_emit) {

    Object sender { "sender", "", nullptr };
    TestEventProvider event_sources { "collection", "", &sender };

    EventSource<::Smp::Bool> eso { "eso1", "", &event_sources,
            ::Smp::PrimitiveTypeKind::PTK_Bool };

    ::Smp::IObject *esiSender = nullptr;
    bool esi_bool = false;
    TestEventConsumer event_sinks { "collection", "", &sender };
    EventSink<::Smp::Bool> esi { "esi1", "desc", &event_sinks, [&](
            ::Smp::IObject *obj, const bool &b) {
        esiSender = obj;
        esi_bool = b;
    }, ::Smp::PrimitiveTypeKind::PTK_Bool };

    eso.Subscribe(&esi);

    eso.Emit(&sender, true);
    EXPECT_EQ(&sender, esiSender);
    EXPECT_EQ(true, esi_bool);
    eso.Emit(nullptr, false);
    EXPECT_EQ(nullptr, esiSender);
    EXPECT_EQ(false, esi_bool);

    eso.Unsubscribe(&esi);
    eso.Emit(&sender, true);
    EXPECT_EQ(nullptr, esiSender);
    EXPECT_EQ(false, esi_bool);

}

TEST(EventsTest, exceptions) {

    Object sender { "sender", "", nullptr };
    TestEventProvider event_sources { "collection", "", &sender };

    EventSource<::Smp::Bool> eso { "eso1", "", &event_sources,
            ::Smp::PrimitiveTypeKind::PTK_Bool };

    TestEventConsumer event_sinks { "collection", "", &sender };
    EventSink<::Smp::Bool> esi { "esi1", "desc", &event_sinks, [&](
            ::Smp::IObject*, const bool&) {
    }, ::Smp::PrimitiveTypeKind::PTK_Bool };

    EXPECT_THROW(eso.Unsubscribe(&esi), ::Smp::EventSinkNotSubscribed);
    eso.Subscribe(&esi);
    EXPECT_THROW(eso.Subscribe(&esi), ::Smp::EventSinkAlreadySubscribed);

    EventSink<::Smp::Int8> esi2 { "esi2", "desc", &event_sinks, [&](
            ::Smp::IObject*, const ::Smp::Int8&) {
    }, ::Smp::PrimitiveTypeKind::PTK_Int8 };

    EXPECT_THROW(eso.Subscribe(&esi2), ::Smp::InvalidEventSink);

    EventSink<void> esi3 { "esi3", "desc", &event_sinks, [&](::Smp::IObject*) {
    } };

    EXPECT_THROW(
            esi3.Notify(&sender, { ::Smp::PrimitiveTypeKind::PTK_Bool, true }),
            ::Smp::InvalidAnyType);

}

} // namespace Xsmp
