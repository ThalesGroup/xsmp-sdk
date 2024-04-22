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

#include "Xsmp/Component.h"
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
#include <Smp/Publication/InvalidPrimitiveType.h>
#include <Smp/Publication/TypeAlreadyRegistered.h>
#include <Smp/Publication/TypeNotRegistered.h>
#include <Smp/ReferenceFull.h>
#include <Smp/Services/EntryPointAlreadySubscribed.h>
#include <Smp/Services/EntryPointNotSubscribed.h>
#include <Smp/Services/InvalidCycleTime.h>
#include <Smp/Services/InvalidEventId.h>
#include <Smp/Services/InvalidEventName.h>
#include <Smp/Services/InvalidEventTime.h>
#include <Smp/Services/InvalidSimulationTime.h>
#include <Smp/SimulatorStateKind.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Smp/VoidOperation.h>
#include <Xsmp/Container.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/EventSink.h>
#include <Xsmp/EventSource.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Factory.h>
#include <Xsmp/Field.h>
#include <Xsmp/Model.h>
#include <Xsmp/Object.h>
#include <Xsmp/Publication/Field.h>
#include <Xsmp/Publication/Operation.h>
#include <Xsmp/Publication/Type.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <Xsmp/Reference.h>
#include <gtest/gtest.h>
#include <memory>

namespace Xsmp::Exception {

TEST(Exception, Exception) {

  Object sender{"name"};
  try {
    throwException(&sender, "CustomException", "desc", "error");
    FAIL();
  } catch (const Smp::Exception &e) {
    EXPECT_STREQ(e.GetName(), "CustomException");
    EXPECT_STREQ(e.GetDescription(), "desc");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(), "error");
    EXPECT_STREQ(e.what(), "CustomException(desc): error");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, FieldAlreadyConnected) {

  Object sender{"name"};

  Xsmp::Publication::TypeRegistry typeRegistry;
  Field<bool>::output source{&typeRegistry, Smp::Uuids::Uuid_Bool, "source"};
  Field<bool> target{&typeRegistry, Smp::Uuids::Uuid_Bool, "target"};
  try {
    throwFieldAlreadyConnected(&sender, &source, &target);
    FAIL();
  } catch (const Smp::FieldAlreadyConnected &e) {
    EXPECT_STREQ(e.GetName(), "FieldAlreadyConnected");
    EXPECT_STREQ(e.GetDescription(), "Cannot connect a target field to a data "
                                     "flow field that is already connected");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(
        e.GetMessage(),
        "Fields '<null>.target' and '<null>.source' are already connected.");

    EXPECT_EQ(e.GetSource(), &source);
    EXPECT_EQ(e.GetTarget(), &target);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidTarget) {

  Object sender{"name"};
  Xsmp::Publication::TypeRegistry typeRegistry;
  Field<bool>::output source{&typeRegistry, Smp::Uuids::Uuid_Bool, "source"};
  Field<bool> target{&typeRegistry, Smp::Uuids::Uuid_Bool, "target"};
  try {
    throwInvalidTarget(&sender, &source, &target);
    FAIL();
  } catch (const Smp::InvalidTarget &e) {
    EXPECT_STREQ(e.GetName(), "InvalidTarget");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot connect two data flow fields of incompatible types");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(), "The type of the target '<null>.target' is "
                                 "not compatible with '<null>.source'.");

    EXPECT_EQ(e.GetSource(), &source);
    EXPECT_EQ(e.GetTarget(), &target);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, CannotStore) {

  Object sender{"name"};

  try {
    throwCannotStore(&sender, "test");
    FAIL();
  } catch (const Smp::CannotStore &e) {
    EXPECT_STREQ(e.GetName(), "CannotStore");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is raised when the component cannot store its "
                 "data to the storage writer given to the Store() method");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(), "test");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, CannotRestore) {

  Object sender{"name"};

  try {
    throwCannotRestore(&sender, "test");
    FAIL();
  } catch (const Smp::CannotRestore &e) {
    EXPECT_STREQ(e.GetName(), "CannotRestore");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is raised when the content of the storage "
                 "reader passed to the Restore() method contains invalid data");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(), "test");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidObjectName) {

  Object sender{"name"};

  try {
    throwInvalidObjectName(&sender, "test", "description");
    FAIL();
  } catch (const Smp::InvalidObjectName &e) {
    EXPECT_STREQ(e.GetName(), "InvalidObjectName");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot set an object's name to an invalid name");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(),
                 "The object's name 'test' is invalid. description");

    EXPECT_STREQ(e.GetInvalidName(), "test");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidObjectNameNull) {

  Object sender{"name"};

  try {
    throwInvalidObjectName(&sender, nullptr, "description");
    FAIL();
  } catch (const Smp::InvalidObjectName &e) {
    EXPECT_STREQ(e.GetName(), "InvalidObjectName");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot set an object's name to an invalid name");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(),
                 "The object's name '<null>' is invalid. description");

    EXPECT_STREQ(e.GetInvalidName(), "<null>");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, ContainerFull) {

  Object parent{"parent"};

  Xsmp::Container<Smp::IObject> ctn{"ctn", "", &parent, 0, 0};
  try {
    throwContainerFull(&ctn);
    FAIL();
  } catch (const Smp::ContainerFull &e) {
    EXPECT_STREQ(e.GetName(), "ContainerFull");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot add a component to a container that is full, i.e. "
                 "where the Count has reached the Upper limit");
    EXPECT_EQ(e.GetSender(), &ctn);
    EXPECT_STREQ(
        e.GetMessage(),
        "The Container 'ctn' in '<null>.parent' is full, upper limit is '0'.");

    EXPECT_EQ(e.GetContainerSize(), 0);
    EXPECT_STREQ(e.GetContainerName(), "ctn");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, DuplicateName) {

  Object sender{"name"};

  try {
    throwDuplicateName(&sender, "test", &sender);
    FAIL();
  } catch (const Smp::DuplicateName &e) {
    EXPECT_STREQ(e.GetName(), "DuplicateName");
    EXPECT_STREQ(
        e.GetDescription(),
        "Cannot add an object to a collection of objects, which have to have "
        "unique names, but another object with the same name does exist "
        "already in this collection. This would lead to duplicate names");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(),
                 "Tried to add an object named 'test' in '<null>.name' "
                 "collection already containing an object with this name.");

    EXPECT_STREQ(e.GetDuplicateName(), "test");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, NotContained) {

  Object parent{"parent"};

  Xsmp::Container<Smp::IObject> ctn{"ctn", "", &parent, 0, 0};
  Xsmp::Model cmp{"cmp"};
  try {
    throwNotContained(&ctn, &cmp);
    FAIL();
  } catch (const Smp::NotContained &e) {
    EXPECT_STREQ(e.GetName(), "NotContained");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is thrown when trying to delete a component "
                 "from a container which was not contained before");
    EXPECT_EQ(e.GetSender(), &ctn);
    EXPECT_STREQ(e.GetMessage(), "Cannot delete component '<null>/cmp' from "
                                 "the container 'ctn' in '<null>.parent'.");

    EXPECT_STREQ(e.GetContainerName(), "ctn");
    EXPECT_EQ(e.GetComponent(), &cmp);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, CannotDelete) {

  Object parent{"parent"};

  Xsmp::Container<Smp::IObject> ctn{"ctn", "", &parent, 42, 43};
  Xsmp::Model cmp{"cmp"};
  try {
    throwCannotDelete(&ctn, &cmp);
    FAIL();
  } catch (const Smp::CannotDelete &e) {
    EXPECT_STREQ(e.GetName(), "CannotDelete");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is thrown when trying to delete a component "
                 "from a container when the number of contained components is "
                 "lower than or equal to the Lower limit");
    EXPECT_EQ(e.GetSender(), &ctn);
    EXPECT_STREQ(
        e.GetMessage(),
        "Tried to delete <null>/cmp component from the container 'ctn' "
        "in '<null>.parent' but the number of contained components is lower "
        "than or equal to the Lower limit: 42");

    EXPECT_STREQ(e.GetContainerName(), "ctn");
    EXPECT_EQ(e.GetComponent(), &cmp);
    EXPECT_EQ(e.GetLowerLimit(), 42);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidComponentState) {

  Xsmp::Model cmp{"cmp"};
  try {
    throwInvalidComponentState(&cmp, Smp::ComponentStateKind::CSK_Connected,
                               Smp::ComponentStateKind::CSK_Created);
    FAIL();
  } catch (const Smp::InvalidComponentState &e) {
    EXPECT_STREQ(e.GetName(), "InvalidComponentState");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is raised by a component when one of its "
                 "methods is called in an invalid state");
    EXPECT_EQ(e.GetSender(), &cmp);
    EXPECT_STREQ(e.GetMessage(), "The ComponentState of <null>/cmp is in "
                                 "Connected state instead of Created state.");

    EXPECT_EQ(e.GetExpectedState(), Smp::ComponentStateKind::CSK_Created);
    EXPECT_EQ(e.GetInvalidState(), Smp::ComponentStateKind::CSK_Connected);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidObjectType) {
  Object sender{"parent"};
  Xsmp::Model cmp{"cmp"};
  try {
    throwInvalidObjectType<bool>(&sender, &cmp);
    FAIL();
  } catch (const Smp::InvalidObjectType &e) {
    EXPECT_STREQ(e.GetName(), "InvalidObjectType");
    EXPECT_STREQ(e.GetDescription(), "Cannot pass an object of wrong type");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(),
                 "<null>/cmp of type Xsmp::Model cannot be casted to bool.");

    EXPECT_EQ(e.GetInvalidObject(), &cmp);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, NotReferenced) {

  Object parent{"parent"};

  Xsmp::Reference<Smp::IObject> ctn{"ctn", "", &parent, 0, 0};
  Xsmp::Model cmp{"cmp"};
  try {
    throwNotReferenced(&ctn, &cmp);
    FAIL();
  } catch (const Smp::NotReferenced &e) {
    EXPECT_STREQ(e.GetName(), "NotReferenced");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is thrown when trying to remove a component "
                 "from a reference which was not referenced before");
    EXPECT_EQ(e.GetSender(), &ctn);
    EXPECT_STREQ(e.GetMessage(),
                 "Tried to remove <null>/cmp component from the reference "
                 "'ctn' in '<null>.parent' which was not referenced before.");

    EXPECT_STREQ(e.GetReferenceName(), "ctn");
    EXPECT_EQ(e.GetComponent(), &cmp);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, ReferenceFull) {

  Object parent{"parent"};

  Xsmp::Reference<Smp::IObject> ctn{"ctn", "", &parent, 0, 0};

  try {
    throwReferenceFull(&ctn, 42);
    FAIL();
  } catch (const Smp::ReferenceFull &e) {
    EXPECT_STREQ(e.GetName(), "ReferenceFull");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot add a component to a reference that is full, i.e. "
                 "where the Count has reached the Upper limit");
    EXPECT_EQ(e.GetSender(), &ctn);
    EXPECT_STREQ(e.GetMessage(), "Tried to add a component to reference 'ctn' "
                                 "in '<null>.parent' that is full, upper"
                                 " limit is '42'.");

    EXPECT_STREQ(e.GetReferenceName(), "ctn");
    EXPECT_EQ(e.GetReferenceSize(), 42);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, CannotRemove) {

  Object parent{"parent"};

  Xsmp::Reference<Smp::IObject> ctn{"ctn", "", &parent, 0, 0};
  Xsmp::Model cmp{"cmp"};
  try {
    throwCannotRemove(&ctn, &cmp, 42);
    FAIL();
  } catch (const Smp::CannotRemove &e) {
    EXPECT_STREQ(e.GetName(), "CannotRemove");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is thrown when trying to remove a component "
                 "from a reference when the number of referenced components is "
                 "lower than or equal to the Lower limit");
    EXPECT_EQ(e.GetSender(), &ctn);
    EXPECT_STREQ(e.GetMessage(),
                 "Tried to remove <null>/cmp component from reference 'ctn' in "
                 "'<null>.parent'"
                 " but the number of referenced components is lower "
                 "than or equal to the Lower limit: 42");

    EXPECT_STREQ(e.GetReferenceName(), "ctn");
    EXPECT_EQ(e.GetComponent(), &cmp);
    EXPECT_EQ(e.GetLowerLimit(), 42);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidEventSink) {

  Object parent{"parent"};

  EventSource<void> eso{"eso1", "", &parent};

  EventSink<void> esi{"esi1", "desc", &parent, [](::Smp::IObject *) {}};

  try {
    throwInvalidEventSink(&parent, &eso, &esi);
    FAIL();
  } catch (const Smp::InvalidEventSink &e) {
    EXPECT_STREQ(e.GetName(), "InvalidEventSink");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot subscribe an event sink to an event source that has a "
                 "different event type");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(),
                 "Tried to subscribe <null>.parent.eso1 event source to "
                 "<null>.parent.esi1 event sink which has a different type.");

    EXPECT_EQ(e.GetEventSink(), &esi);
    EXPECT_EQ(e.GetEventSource(), &eso);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, EventSinkAlreadySubscribed) {
  Object parent{"parent"};
  EventSource<void> eso{"eso1", "", &parent};
  EventSink<void> esi{"esi1", "desc", &parent, [&](::Smp::IObject *obj) {}};

  try {
    throwEventSinkAlreadySubscribed(&parent, &eso, &esi);
    FAIL();
  } catch (const Smp::EventSinkAlreadySubscribed &e) {
    EXPECT_STREQ(e.GetName(), "EventSinkAlreadySubscribed");
    EXPECT_STREQ(e.GetDescription(), "Cannot subscribe an event sink to an "
                                     "event source that is already subscribed");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(),
                 "Tried to subscribe <null>.parent.eso1 event source to "
                 "<null>.parent.esi1 event sink that is already subscribed.");

    EXPECT_EQ(e.GetEventSink(), &esi);
    EXPECT_EQ(e.GetEventSource(), &eso);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, EventSinkNotSubscribed) {

  Object parent{"parent"};
  EventSource<void> eso{"eso1", "", &parent};
  EventSink<void> esi{"esi1", "desc", &parent, [&](::Smp::IObject *obj) {}};

  try {
    throwEventSinkNotSubscribed(&parent, &eso, &esi);
    FAIL();
  } catch (const Smp::EventSinkNotSubscribed &e) {
    EXPECT_STREQ(e.GetName(), "EventSinkNotSubscribed");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot unsubscribe an event sink from an event source that "
                 "is not subscribed to it");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(
        e.GetMessage(),
        "Tried to unsubscribe <null>.parent.esi1 event sink from "
        "<null>.parent.eso1 event source that is not subscribed to it.");

    EXPECT_EQ(e.GetEventSink(), &esi);
    EXPECT_EQ(e.GetEventSource(), &eso);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidOperationName) {

  Object parent{"parent"};

  try {
    throwInvalidOperationName(&parent, "op");
    FAIL();
  } catch (const Smp::InvalidOperationName &e) {
    EXPECT_STREQ(e.GetName(), "InvalidOperationName");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is raised by the Invoke() method when trying "
                 "to invoke a method that does not exist, or that does not "
                 "support dynamic invocation");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(),
                 "Tried to invoke a method that does not exist, or that does "
                 "not support dynamic invocation in operation: op");

    EXPECT_STREQ(e.GetOperationName(), "op");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidParameterCount) {

  Xsmp::Publication::Operation parent{"op"};

  try {
    throwInvalidParameterCount(&parent, 42);
    FAIL();
  } catch (const Smp::InvalidParameterCount &e) {
    EXPECT_STREQ(e.GetName(), "InvalidParameterCount");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is raised by the Invoke() method when trying "
                 "to invoke a method with a wrong number of parameters");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(), "Tried to invoke operation <null>.op with 42 "
                                 "parameter(s) instead of 0.");

    EXPECT_STREQ(e.GetOperationName(), "op");
    EXPECT_EQ(e.GetRequestParameters(), 42);
    EXPECT_EQ(e.GetOperationParameters(), 0);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidParameterType) {

  Xsmp::Object sender{"sender"};

  try {
    throwInvalidParameterType(&sender, "op", "parameter",
                              ::Smp::PrimitiveTypeKind::PTK_Bool,
                              ::Smp::PrimitiveTypeKind::PTK_Float32);
    FAIL();
  } catch (const Smp::InvalidParameterType &e) {
    EXPECT_STREQ(e.GetName(), "InvalidParameterType");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is raised by the Invoke() method when trying "
                 "to invoke a method passing a parameter of wrong type");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(),
                 "In operation op, tried to pass parameter parameter which is "
                 "of type Bool instead of expected type Float32");

    EXPECT_STREQ(e.GetOperationName(), "op");
    EXPECT_STREQ(e.GetParameterName(), "parameter");
    EXPECT_EQ(e.GetExpectedType(), ::Smp::PrimitiveTypeKind::PTK_Float32);
    EXPECT_EQ(e.GetInvalidType(), ::Smp::PrimitiveTypeKind::PTK_Bool);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidArrayIndex) {

  Xsmp::Publication::TypeRegistry registry;

  const auto *type = dynamic_cast<Xsmp::Publication::ArrayType *>(
      registry.AddArrayType("type", "", Smp::Uuid{}, Smp::Uuids::Uuid_Bool,
                            sizeof(bool), 10, false));

  Xsmp::Publication::ArrayField sender{"field", "",    nullptr,
                                       nullptr, type,  Smp::ViewKind::VK_All,
                                       true,    false, false};

  try {
    throwInvalidArrayIndex(&sender, 42);
    FAIL();
  } catch (const Smp::InvalidArrayIndex &e) {
    EXPECT_STREQ(e.GetName(), "InvalidArrayIndex");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is raised when an invalid index is specified");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(), "The index '42' is negative or exceeds the "
                                 "array size '10' of '<null>.field'.");

    EXPECT_EQ(e.GetInvalidIndex(), 42);
    EXPECT_EQ(e.GetArraySize(), 10);
  } catch (...) {
    FAIL();
  }

  Xsmp::Publication::SimpleArrayField simple{
      "field", "",    nullptr, nullptr, type, Smp::ViewKind::VK_All,
      true,    false, false};

  try {
    throwInvalidArrayIndex(&simple, -1);
    FAIL();
  } catch (const Smp::InvalidArrayIndex &e) {
    EXPECT_STREQ(e.GetName(), "InvalidArrayIndex");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is raised when an invalid index is specified");
    EXPECT_EQ(e.GetSender(), &simple);
    EXPECT_STREQ(e.GetMessage(), "The index '-1' is negative or exceeds the "
                                 "array size '10' of '<null>.field'.");

    EXPECT_EQ(e.GetInvalidIndex(), -1);
    EXPECT_EQ(e.GetArraySize(), 10);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidFieldValue) {

  const Object sender{"name"};
  Xsmp::Publication::TypeRegistry typeRegistry;
  Field<bool> field{&typeRegistry, Smp::Uuids::Uuid_Bool, "field"};

  auto invalidValue =
      Smp::AnySimple{::Smp::PrimitiveTypeKind::PTK_Float32, 42.};
  try {
    throwInvalidFieldValue(&field, invalidValue);
    FAIL();
  } catch (const Smp::InvalidFieldValue &e) {
    EXPECT_STREQ(e.GetName(), "InvalidFieldValue");
    EXPECT_STREQ(e.GetDescription(), "This exception is raised when trying to "
                                     "assign an illegal value to a field");
    EXPECT_EQ(e.GetSender(), &field);
    EXPECT_STREQ(e.GetMessage(), "Cannot assign value '42f32' to the field "
                                 "<null>.field of Bool's type.");

    EXPECT_EQ(e.GetInvalidFieldValue(), invalidValue);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidArrayValue) {

  Xsmp::Publication::TypeRegistry registry;

  const auto *type = dynamic_cast<Xsmp::Publication::ArrayType *>(
      registry.AddArrayType("Test", "", Smp::Uuid{}, Smp::Uuids::Uuid_Bool,
                            sizeof(bool), 10, false));

  Xsmp::Publication::SimpleArrayField sender{
      "field", "",    nullptr, nullptr, type, Smp::ViewKind::VK_All,
      true,    false, false};

  auto invalidValue =
      Smp::AnySimple{::Smp::PrimitiveTypeKind::PTK_Float32, 42.};
  try {
    throwInvalidArrayValue(&sender, 4, invalidValue);
    FAIL();
  } catch (const Smp::InvalidArrayValue &e) {
    EXPECT_STREQ(e.GetName(), "InvalidArrayValue");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is raised when trying to assign an illegal "
                 "value to an array field");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(), "Cannot assign value '42f32' to the field "
                                 "<null>.field[4] of Test's type.");

    EXPECT_EQ(e.GetInvalidValue(), invalidValue);
    EXPECT_EQ(e.GetInvalidValueIndex(), 4);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidArraySize) {

  Xsmp::Publication::TypeRegistry registry;

  const auto *type = dynamic_cast<Xsmp::Publication::ArrayType *>(
      registry.AddArrayType("Test", "", Smp::Uuid{}, Smp::Uuids::Uuid_Bool,
                            sizeof(bool), 10, false));

  Xsmp::Publication::SimpleArrayField sender{
      "field", "",    nullptr, nullptr, type, Smp::ViewKind::VK_All,
      true,    false, false};

  try {
    throwInvalidArraySize(&sender, 4);
    FAIL();
  } catch (const Smp::InvalidArraySize &e) {
    EXPECT_STREQ(e.GetName(), "InvalidArraySize");
    EXPECT_STREQ(
        e.GetDescription(),
        "This exception is raised when an invalid array size is specified");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(
        e.GetMessage(),
        "The array size 4 of <null>.field is invalid, expecting 10 elements.");

    EXPECT_EQ(e.GetInvalidSize(), 4);
    EXPECT_EQ(e.GetArraySize(), 10);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidEventName) {

  Object sender{"name"};
  try {
    throwInvalidEventName(&sender, nullptr);
    FAIL();
  } catch (const Smp::Services::InvalidEventName &e) {
    EXPECT_STREQ(e.GetName(), "InvalidEventName");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is thrown by the QueryEventId() method of the "
                 "event manager when an empty event name has been provided");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(), "The Event Name '<null>' is invalid.");
  } catch (...) {
    FAIL();
  }
  try {
    throwInvalidEventName(&sender, "");
    FAIL();
  } catch (const Smp::Services::InvalidEventName &e) {
    EXPECT_STREQ(e.GetName(), "InvalidEventName");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is thrown by the QueryEventId() method of the "
                 "event manager when an empty event name has been provided");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(), "The Event Name '' is invalid.");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidEventId) {

  Object sender{"name"};
  try {
    throwInvalidEventId(&sender, 42);
    FAIL();
  } catch (const Smp::Services::InvalidEventId &e) {
    EXPECT_STREQ(e.GetName(), "InvalidEventId");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is raised when an invalid event id is "
                 "provided, e.g.when calling Subscribe(),"
                 " Unsubscribe() or Emit() of the Event Manager (using an "
                 "invalid global event id), or when calling "
                 "SetEventSimulationTime(), SetEventMissionTime(), "
                 "SetEventEpochTime(), SetEventZuluTime(), SetEventCycleTime(),"
                 " SetEventCount() or RemoveEvent() of the Scheduler (using an "
                 "invalid scheduler event id)");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(), "The EventId '42' is invalid.");
    EXPECT_EQ(e.GetInvalidEventId(), 42);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidCycleTime) {

  Object sender{"name"};
  try {
    throwInvalidCycleTime(&sender, -42);
    FAIL();
  } catch (const Smp::Services::InvalidCycleTime &e) {
    EXPECT_STREQ(e.GetName(), "InvalidCycleTime");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is thrown by one of the AddEvent() methods of "
                 "the scheduler when the event "
                 "is a cyclic event (i.e. repeat is not 0),  but the cycle "
                 "time specified is not a positive duration");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(
        e.GetMessage(),
        "The cycle time '-00:00:00.000000042' is not a positive value.");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidEventTime) {

  Object sender{"name"};
  try {
    throwInvalidEventTime(&sender, 1000, 2000);
    FAIL();
  } catch (const Smp::Services::InvalidEventTime &e) {
    EXPECT_STREQ(e.GetName(), "InvalidEventTime");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is thrown by one of the AddEvent() methods of "
                 "the scheduler when the time specified for the first "
                 "execution of the  event is in the past");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(),
                 "The event time '00:00:00.000001000' is before the current "
                 "time '00:00:00.000002000'.");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, EntryPointNotSubscribed) {
  Object sender{"name"};
  EntryPoint ep{"ep", "", &sender, [] {}};
  try {
    throwEntryPointNotSubscribed(&sender, &ep, "test");
    FAIL();
  } catch (const Smp::Services::EntryPointNotSubscribed &e) {
    EXPECT_STREQ(e.GetName(), "EntryPointNotSubscribed");
    EXPECT_STREQ(e.GetDescription(), "Cannot unsubscribe an entry point from "
                                     "an event that is not subscribed to it");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(),
                 "The EntryPoint <null>.name.ep is not subscribed to 'test'.");
    EXPECT_EQ(e.GetEntryPoint(), &ep);
    EXPECT_STREQ(e.GetEventName(), "test");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, EntryPointAlreadySubscribed) {
  Object sender{"name"};
  EntryPoint ep{"ep", "", &sender, [] {}};
  try {
    throwEntryPointAlreadySubscribed(&sender, &ep, "test");
    FAIL();
  } catch (const Smp::Services::EntryPointAlreadySubscribed &e) {
    EXPECT_STREQ(e.GetName(), "EntryPointAlreadySubscribed");
    EXPECT_STREQ(e.GetDescription(), "Cannot subscribe an entry point to an "
                                     "event that is already subscribed");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(
        e.GetMessage(),
        "The EntryPoint <null>.name.ep is already subscribed to 'test'.");
    EXPECT_EQ(e.GetEntryPoint(), &ep);
    EXPECT_STREQ(e.GetEventName(), "test");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidFieldName) {
  Object sender{"name"};

  try {
    throwInvalidFieldName(&sender, "test");
    FAIL();
  } catch (const Smp::InvalidFieldName &e) {
    EXPECT_STREQ(e.GetName(), "InvalidFieldName");
    EXPECT_STREQ(
        e.GetDescription(),
        "This exception is raised when an invalid field name is specified");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(),
                 "'<null>.name' does no contains a field named 'test'.");
    EXPECT_STREQ(e.GetFieldName(), "test");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidFieldNameNull) {
  Object sender{"name"};

  try {
    throwInvalidFieldName(&sender, nullptr);
    FAIL();
  } catch (const Smp::InvalidFieldName &e) {
    EXPECT_STREQ(e.GetName(), "InvalidFieldName");
    EXPECT_STREQ(
        e.GetDescription(),
        "This exception is raised when an invalid field name is specified");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(),
                 "'<null>.name' does no contains a field named '<null>'.");
    EXPECT_STREQ(e.GetFieldName(), "<null>");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, TypeNotRegistered) {
  Object sender{"name"};
  const Smp::Uuid uuid{0, 1, 2, 3, 4};
  try {
    throwTypeNotRegistered(&sender, uuid);
    FAIL();
  } catch (const Smp::Publication::TypeNotRegistered &e) {
    EXPECT_STREQ(e.GetName(), "TypeNotRegistered");
    EXPECT_STREQ(e.GetDescription(), "Cannot publish a feature with a type "
                                     "Uuid that has not been registered");
    EXPECT_EQ(e.GetSender(), &sender);
    EXPECT_STREQ(e.GetMessage(),
                 "The Uuid '00000000-0001-0002-0003-000000000004' has not been "
                 "registered.");
    EXPECT_EQ(e.GetUuid(), uuid);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidParameterIndex) {

  Xsmp::Publication::Operation parent{"op"};

  try {
    throwInvalidParameterIndex(&parent, 42, 8);
    FAIL();
  } catch (const Smp::InvalidParameterIndex &e) {
    EXPECT_STREQ(e.GetName(), "InvalidParameterIndex");
    EXPECT_STREQ(
        e.GetDescription(),
        "This exception is raised when using an invalid parameter index to set "
        "(SetParameterValue()) or get (GetParameterValue()) a parameter value "
        "of an operation in a request");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(),
                 "The Operation '<null>.op' has '8' parameter(s). Cannot "
                 "access to parameter at index '42'.");

    EXPECT_STREQ(e.GetOperationName(), "op");
    EXPECT_EQ(e.GetParameterCount(), 8);
    EXPECT_EQ(e.GetParameterIndex(), 42);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, VoidOperation) {

  Xsmp::Publication::Operation parent{"op"};

  try {
    throwVoidOperation(&parent);
    FAIL();
  } catch (const Smp::VoidOperation &e) {
    EXPECT_STREQ(e.GetName(), "VoidOperation");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot read (GetReturnValue()) or write (SetReturnValue()) "
                 "the return value of a void operation");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(),
                 "The Operation '<null>.op' does not have a return value");

    EXPECT_STREQ(e.GetOperationName(), "op");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidAnyType) {

  Xsmp::Object parent{"op"};

  try {
    throwInvalidAnyType(&parent, Smp::PrimitiveTypeKind::PTK_Char8,
                        Smp::PrimitiveTypeKind::PTK_Bool);
    FAIL();
  } catch (const Smp::InvalidAnyType &e) {
    EXPECT_STREQ(e.GetName(), "InvalidAnyType");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot use an AnySimple argument of wrong type");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(),
                 "The AnySimple type 'Bool' is invalid: expected 'Char8'.");

    EXPECT_EQ(e.GetInvalidType(), Smp::PrimitiveTypeKind::PTK_Bool);
    EXPECT_EQ(e.GetExpectedType(), Smp::PrimitiveTypeKind::PTK_Char8);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidReturnValue) {

  Xsmp::Publication::Operation parent{"op"};
  const Smp::AnySimple invalidValue{Smp::PrimitiveTypeKind::PTK_Bool, true};
  try {
    throwInvalidReturnValue(&parent, invalidValue);
    FAIL();
  } catch (const Smp::InvalidReturnValue &e) {
    EXPECT_STREQ(e.GetName(), "InvalidReturnValue");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot assign an invalid return value of an operation in a "
                 "request using SetReturnValue()");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(
        e.GetMessage(),
        "The return value 'true' is invalid for Operation '<null>.op'.");

    EXPECT_STREQ(e.GetOperationName(), "op");
    EXPECT_EQ(e.GetValue(), invalidValue);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidParameterValue) {

  Xsmp::Publication::Operation parent{"op"};
  const Smp::AnySimple invalidValue{Smp::PrimitiveTypeKind::PTK_Bool, true};
  try {
    throwInvalidParameterValue(&parent, "param", invalidValue);
    FAIL();
  } catch (const Smp::InvalidParameterValue &e) {
    EXPECT_STREQ(e.GetName(), "InvalidParameterValue");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot assign an illegal value to a parameter of an "
                 "operation in a request using SetParameterValue()");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(), "The value 'true' is invalid for parameter "
                                 "'param' in Operation '<null>.op'.");

    EXPECT_STREQ(e.GetParameterName(), "param");
    EXPECT_EQ(e.GetValue(), invalidValue);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidFieldType) {

  Xsmp::Object parent{"op"};
  try {
    throwInvalidFieldType(&parent, ::Smp::PrimitiveTypeKind::PTK_String8);
    FAIL();
  } catch (const Smp::InvalidFieldType &e) {
    EXPECT_STREQ(e.GetName(), "InvalidFieldType");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot publish a field with invalid type");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(
        e.GetMessage(),
        "The primitive type kind 'String8' is invalid for field '<null>.op'.");
  } catch (...) {
    FAIL();
  }
  try {
    throwInvalidFieldType(&parent, ::Smp::Uuid{});
    FAIL();
  } catch (const Smp::InvalidFieldType &e) {
    EXPECT_STREQ(e.GetName(), "InvalidFieldType");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot publish a field with invalid type");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(),
                 "The type UUID '00000000-0000-0000-0000-000000000000' is "
                 "invalid for field '<null>.op'.");
  } catch (...) {
    FAIL();
  }
  const Xsmp::Publication::TypeRegistry registry;
  try {
    throwInvalidFieldType(
        &parent, registry.GetType(::Smp::PrimitiveTypeKind::PTK_String8));
    FAIL();
  } catch (const Smp::InvalidFieldType &e) {
    EXPECT_STREQ(e.GetName(), "InvalidFieldType");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot publish a field with invalid type");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(),
                 "The type 'String8' is invalid for field '<null>.op'.");
  } catch (...) {
    FAIL();
  }
}

class M1 : public ::Xsmp::Model {
public:
  using ::Xsmp::Model::Model;
  const ::Smp::Uuid &GetUuid() const override { return uuid; }
  static constexpr ::Smp::Uuid uuid{1, 2, 3, 4, 5};
};

TEST(Exception, DuplicateUuid) {
  Xsmp::Object parent{"parent"};
  auto factory = std::unique_ptr<Smp::IFactory>(
      ::Xsmp::Factory::Create<M1>("M1", "M1 model", nullptr, M1::uuid));

  try {
    throwDuplicateUuid(&parent, factory.get(), "M1Duplicate");
    FAIL();
  } catch (const Smp::DuplicateUuid &e) {
    EXPECT_STREQ(e.GetName(), "DuplicateUuid");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot register a factory under a Uuid that has already been "
                 "used to register another (or the same) factory. This would "
                 "lead to duplicate implementation Uuids");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(),
                 "The Uuid '00000001-0002-0003-0004-000000000005' of Factory "
                 "'M1Duplicate' collides with the existing factory 'M1'.");
    EXPECT_STREQ(e.GetOldName(), "M1");
    EXPECT_STREQ(e.GetNewName(), "M1Duplicate");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, LibraryNotFound) {
  Xsmp::Object parent{"parent"};

  try {
    throwLibraryNotFound(&parent, "library", "error");
    FAIL();
  } catch (const Smp::LibraryNotFound &e) {
    EXPECT_STREQ(e.GetName(), "LibraryNotFound");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot load a library that does not exist");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(), "error");
    EXPECT_STREQ(e.GetLibraryName(), "library");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidLibrary) {
  Xsmp::Object parent{"parent"};

  try {
    throwInvalidLibrary(&parent, "library", "error");
    FAIL();
  } catch (const Smp::InvalidLibrary &e) {
    EXPECT_STREQ(e.GetName(), "InvalidLibrary");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot load an undefined symbol from a library");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(), "error");
    EXPECT_STREQ(e.GetLibraryName(), "library");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidSimulationTime) {
  Xsmp::Object parent{"parent"};

  try {
    throwInvalidSimulationTime(&parent, 1, 4, 3);
    FAIL();
  } catch (const Smp::Services::InvalidSimulationTime &e) {
    EXPECT_STREQ(e.GetName(), "InvalidSimulationTime");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is thrown by SetSimulationTime if the new "
                 "simulation time is not between the current simulation time "
                 "and the simulation time of the next event on the scheduler");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(), "The simulation time '4' is invalid. "
                                 "Expecting a value between '1' and '3'.");

    EXPECT_EQ(e.GetCurrentTime(), 1);
    EXPECT_EQ(e.GetProvidedTime(), 4);
    EXPECT_EQ(e.GetMaximumTime(), 3);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, TypeAlreadyRegistered) {
  Xsmp::Object parent{"parent"};

  Xsmp::Publication::TypeRegistry registry;
  auto *type = registry.AddEnumerationType("Enum", "", {}, sizeof(Smp::Int32));

  try {
    throwTypeAlreadyRegistered(&parent, "OtherType", type);
    FAIL();
  } catch (const Smp::Publication::TypeAlreadyRegistered &e) {
    EXPECT_STREQ(e.GetName(), "TypeAlreadyRegistered");
    EXPECT_STREQ(
        e.GetDescription(),
        "Cannot register a type with a Uuid that has already been registered");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(), "Cannot register 'OtherType' with Uuid "
                                 "'00000000-0000-0000-0000-000000000000'. This "
                                 "Uuid has already been registered.");

    EXPECT_EQ(e.GetType(), type);
    EXPECT_STREQ(e.GetTypeName(), "OtherType");
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidPrimitiveType) {
  Xsmp::Object parent{"parent"};

  try {
    throwInvalidPrimitiveType(&parent, "OtherType",
                              Smp::PrimitiveTypeKind::PTK_Bool);
    FAIL();
  } catch (const Smp::Publication::InvalidPrimitiveType &e) {
    EXPECT_STREQ(e.GetName(), "InvalidPrimitiveType");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot use an invalid primitive type kind as parameter for a "
                 "user-defined float or integer type");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(),
                 "The primitive type 'Bool' is invalid for 'OtherType'.");

    EXPECT_STREQ(e.GetTypeName(), "OtherType");
    EXPECT_EQ(e.GetType(), Smp::PrimitiveTypeKind::PTK_Bool);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, InvalidSimulatorState) {
  Xsmp::Object parent{"parent"};

  try {
    throwInvalidSimulatorState(&parent, Smp::SimulatorStateKind::SSK_Executing);
    FAIL();
  } catch (const Smp::InvalidSimulatorState &e) {
    EXPECT_STREQ(e.GetName(), "InvalidSimulatorState");
    EXPECT_STREQ(e.GetDescription(),
                 "This exception is raised by the simulator when one of the "
                 "operations is called in an invalid state");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(), "Cannot performs an operation while the "
                                 "simulator is in 'Executing' state.");

    EXPECT_EQ(e.GetInvalidState(), Smp::SimulatorStateKind::SSK_Executing);
  } catch (...) {
    FAIL();
  }
}

TEST(Exception, DuplicateLiteral) {
  Xsmp::Object parent{"parent"};

  try {
    throwDuplicateLiteral(&parent, "test", 2);
    FAIL();
  } catch (const Smp::Publication::DuplicateLiteral &e) {
    EXPECT_STREQ(e.GetName(), "DuplicateLiteral");
    EXPECT_STREQ(e.GetDescription(),
                 "Cannot add a literal to an enumeration using a value that "
                 "has been used for another literal before.");
    EXPECT_EQ(e.GetSender(), &parent);
    EXPECT_STREQ(e.GetMessage(),
                 "Value '2' is already used by 'test' literal.");

    EXPECT_EQ(e.GetLiteralValue(), 2);
    EXPECT_STREQ(e.GetLiteralName(), "test");
  } catch (...) {
    FAIL();
  }
}
} // namespace Xsmp::Exception
