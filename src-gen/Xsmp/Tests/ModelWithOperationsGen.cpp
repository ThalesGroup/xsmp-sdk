// Copyright 2023-2024 THALES ALENIA SPACE FRANCE. All rights reserved.
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
// -----------------------------------------------------------------------------
// File Name    : ModelWithOperationsGen.cpp
// Generated by : XsmpSdkGenerator-1.1.4
// -----------------------------------------------------------------------------
/// @file Xsmp/Tests/ModelWithOperationsGen.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------
// ----------------------------------------------------------------------------

#include <Smp/IPublication.h>
#include <Xsmp/ComponentHelper.h>
#include <Xsmp/Request.h>
#include <Xsmp/Tests/ModelWithOperations.h>

namespace Xsmp::Tests {
//--------------------------- Constructor -------------------------
ModelWithOperationsGen::ModelWithOperationsGen(::Smp::String8 name,
                                               ::Smp::String8 description,
                                               ::Smp::IComposite *parent,
                                               ::Smp::ISimulator *simulator)
    : // Base class initialization
      ::Xsmp::Model(name, description, parent, simulator) {}

void ModelWithOperationsGen::Publish(::Smp::IPublication *receiver) {
  // Call parent class implementation first
  ::Xsmp::Model::Publish(receiver);

  {
    // Publish operation voidOperation
    receiver->PublishOperation("voidOperation", "", ::Smp::ViewKind::VK_None);
  }
  {
    // Publish operation boolOperation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("boolOperation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_Bool,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation char8Operation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("char8Operation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_Char8,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation dateTimeOperation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("dateTimeOperation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_DateTime,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation durationOperation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("durationOperation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_Duration,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation float32Operation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("float32Operation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_Float32,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation float64Operation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("float64Operation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_Float64,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation int16Operation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("int16Operation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_Int16,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation int32Operation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("int32Operation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_Int32,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation int64Operation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("int64Operation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_Int64,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation int8Operation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("int8Operation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_Int8,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation string8Operation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("string8Operation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_String8,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation uint16Operation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("uint16Operation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_UInt16,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation uint32Operation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("uint32Operation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_UInt32,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation uint64Operation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("uint64Operation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_UInt64,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  {
    // Publish operation uint8Operation
    ::Smp::Publication::IPublishOperation *operation =
        receiver->PublishOperation("uint8Operation", "",
                                   ::Smp::ViewKind::VK_None);
    operation->PublishParameter(
        "return", "", ::Smp::Uuids::Uuid_UInt8,
        Smp::Publication::ParameterDirectionKind::PDK_Return);
  }
  // Call user DoPublish if any
  ::Xsmp::Component::Helper::Publish<::Xsmp::Tests::ModelWithOperations>(
      this, receiver);
}

void ModelWithOperationsGen::Configure(
    ::Smp::Services::ILogger *logger,
    ::Smp::Services::ILinkRegistry *linkRegistry) {
  // Call parent implementation first
  ::Xsmp::Model::Configure(logger, linkRegistry);

  // Call user DoConfigure if any
  ::Xsmp::Component::Helper::Configure<::Xsmp::Tests::ModelWithOperations>(
      this, logger, linkRegistry);
}

void ModelWithOperationsGen::Connect(::Smp::ISimulator *simulator) {
  // Call parent implementation first
  ::Xsmp::Model::Connect(simulator);

  // Call user DoConnect if any
  ::Xsmp::Component::Helper::Connect<::Xsmp::Tests::ModelWithOperations>(
      this, simulator);
}

void ModelWithOperationsGen::Disconnect() {
  // Call user DoDisconnect if any
  ::Xsmp::Component::Helper::Disconnect<::Xsmp::Tests::ModelWithOperations>(
      this);

  // Call parent implementation last, to remove references to the Simulator and
  // its services
  ::Xsmp::Model::Disconnect();
}

ModelWithOperationsGen::RequestHandlers
    ModelWithOperationsGen::requestHandlers = InitRequestHandlers();

ModelWithOperationsGen::RequestHandlers
ModelWithOperationsGen::InitRequestHandlers() {
  RequestHandlers handlers;
  if (handlers.find("voidOperation") == handlers.end()) {
    handlers["voidOperation"] = [](ModelWithOperationsGen *cmp,
                                   ::Smp::IRequest *) {
      /// Invoke voidOperation
      cmp->voidOperation();
    };
  }
  if (handlers.find("boolOperation") == handlers.end()) {
    handlers["boolOperation"] = [](ModelWithOperationsGen *cmp,
                                   ::Smp::IRequest *req) {
      /// Invoke boolOperation
      const auto p_return = cmp->boolOperation();

      ::Xsmp::Request::setReturnValue(req, ::Smp::PrimitiveTypeKind::PTK_Bool,
                                      p_return);
    };
  }
  if (handlers.find("char8Operation") == handlers.end()) {
    handlers["char8Operation"] = [](ModelWithOperationsGen *cmp,
                                    ::Smp::IRequest *req) {
      /// Invoke char8Operation
      const auto p_return = cmp->char8Operation();

      ::Xsmp::Request::setReturnValue(req, ::Smp::PrimitiveTypeKind::PTK_Char8,
                                      p_return);
    };
  }
  if (handlers.find("dateTimeOperation") == handlers.end()) {
    handlers["dateTimeOperation"] = [](ModelWithOperationsGen *cmp,
                                       ::Smp::IRequest *req) {
      /// Invoke dateTimeOperation
      const auto p_return = cmp->dateTimeOperation();

      ::Xsmp::Request::setReturnValue(
          req, ::Smp::PrimitiveTypeKind::PTK_DateTime, p_return);
    };
  }
  if (handlers.find("durationOperation") == handlers.end()) {
    handlers["durationOperation"] = [](ModelWithOperationsGen *cmp,
                                       ::Smp::IRequest *req) {
      /// Invoke durationOperation
      const auto p_return = cmp->durationOperation();

      ::Xsmp::Request::setReturnValue(
          req, ::Smp::PrimitiveTypeKind::PTK_Duration, p_return);
    };
  }
  if (handlers.find("float32Operation") == handlers.end()) {
    handlers["float32Operation"] = [](ModelWithOperationsGen *cmp,
                                      ::Smp::IRequest *req) {
      /// Invoke float32Operation
      const auto p_return = cmp->float32Operation();

      ::Xsmp::Request::setReturnValue(
          req, ::Smp::PrimitiveTypeKind::PTK_Float32, p_return);
    };
  }
  if (handlers.find("float64Operation") == handlers.end()) {
    handlers["float64Operation"] = [](ModelWithOperationsGen *cmp,
                                      ::Smp::IRequest *req) {
      /// Invoke float64Operation
      const auto p_return = cmp->float64Operation();

      ::Xsmp::Request::setReturnValue(
          req, ::Smp::PrimitiveTypeKind::PTK_Float64, p_return);
    };
  }
  if (handlers.find("int16Operation") == handlers.end()) {
    handlers["int16Operation"] = [](ModelWithOperationsGen *cmp,
                                    ::Smp::IRequest *req) {
      /// Invoke int16Operation
      const auto p_return = cmp->int16Operation();

      ::Xsmp::Request::setReturnValue(req, ::Smp::PrimitiveTypeKind::PTK_Int16,
                                      p_return);
    };
  }
  if (handlers.find("int32Operation") == handlers.end()) {
    handlers["int32Operation"] = [](ModelWithOperationsGen *cmp,
                                    ::Smp::IRequest *req) {
      /// Invoke int32Operation
      const auto p_return = cmp->int32Operation();

      ::Xsmp::Request::setReturnValue(req, ::Smp::PrimitiveTypeKind::PTK_Int32,
                                      p_return);
    };
  }
  if (handlers.find("int64Operation") == handlers.end()) {
    handlers["int64Operation"] = [](ModelWithOperationsGen *cmp,
                                    ::Smp::IRequest *req) {
      /// Invoke int64Operation
      const auto p_return = cmp->int64Operation();

      ::Xsmp::Request::setReturnValue(req, ::Smp::PrimitiveTypeKind::PTK_Int64,
                                      p_return);
    };
  }
  if (handlers.find("int8Operation") == handlers.end()) {
    handlers["int8Operation"] = [](ModelWithOperationsGen *cmp,
                                   ::Smp::IRequest *req) {
      /// Invoke int8Operation
      const auto p_return = cmp->int8Operation();

      ::Xsmp::Request::setReturnValue(req, ::Smp::PrimitiveTypeKind::PTK_Int8,
                                      p_return);
    };
  }
  if (handlers.find("string8Operation") == handlers.end()) {
    handlers["string8Operation"] = [](ModelWithOperationsGen *cmp,
                                      ::Smp::IRequest *req) {
      /// Invoke string8Operation
      const auto p_return = cmp->string8Operation();

      ::Xsmp::Request::setReturnValue(
          req, ::Smp::PrimitiveTypeKind::PTK_String8, p_return);
    };
  }
  if (handlers.find("uint16Operation") == handlers.end()) {
    handlers["uint16Operation"] = [](ModelWithOperationsGen *cmp,
                                     ::Smp::IRequest *req) {
      /// Invoke uint16Operation
      const auto p_return = cmp->uint16Operation();

      ::Xsmp::Request::setReturnValue(req, ::Smp::PrimitiveTypeKind::PTK_UInt16,
                                      p_return);
    };
  }
  if (handlers.find("uint32Operation") == handlers.end()) {
    handlers["uint32Operation"] = [](ModelWithOperationsGen *cmp,
                                     ::Smp::IRequest *req) {
      /// Invoke uint32Operation
      const auto p_return = cmp->uint32Operation();

      ::Xsmp::Request::setReturnValue(req, ::Smp::PrimitiveTypeKind::PTK_UInt32,
                                      p_return);
    };
  }
  if (handlers.find("uint64Operation") == handlers.end()) {
    handlers["uint64Operation"] = [](ModelWithOperationsGen *cmp,
                                     ::Smp::IRequest *req) {
      /// Invoke uint64Operation
      const auto p_return = cmp->uint64Operation();

      ::Xsmp::Request::setReturnValue(req, ::Smp::PrimitiveTypeKind::PTK_UInt64,
                                      p_return);
    };
  }
  if (handlers.find("uint8Operation") == handlers.end()) {
    handlers["uint8Operation"] = [](ModelWithOperationsGen *cmp,
                                    ::Smp::IRequest *req) {
      /// Invoke uint8Operation
      const auto p_return = cmp->uint8Operation();

      ::Xsmp::Request::setReturnValue(req, ::Smp::PrimitiveTypeKind::PTK_UInt8,
                                      p_return);
    };
  }
  return handlers;
}

void ModelWithOperationsGen::Invoke(::Smp::IRequest *request) {
  if (request == nullptr) {
    return;
  }
  auto handler = requestHandlers.find(request->GetOperationName());
  if (handler != requestHandlers.end()) {
    handler->second(this, request);
  } else {
    // pass the request down to the base model
    ::Xsmp::Model::Invoke(request);
  }
}

const Smp::Uuid &ModelWithOperationsGen::GetUuid() const {
  return Uuid_ModelWithOperations;
}
} // namespace Xsmp::Tests
