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
// File Name    : ModelWithPropertiesGen.h
// Generated by : XsmpSdkGenerator-1.1.0
// -----------------------------------------------------------------------------
/// @file Xsmp/Tests/ModelWithPropertiesGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef XSMP_TESTS_MODELWITHPROPERTIESGEN_H_
#define XSMP_TESTS_MODELWITHPROPERTIESGEN_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------
// ----------------------------------------------------------------------------

#include <Smp/IRequest.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Uuid.h>
#include <Xsmp/Model.h>
#include <Xsmp/Tests/Types/Enum1.h>
#include <Xsmp/Tests/Types/String1.h>
#include <functional>
#include <map>
#include <type_traits>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------
// ----------------------------------------------------------------------------

namespace Xsmp::Tests {
// forward declaration of user class
class ModelWithProperties;
/// Universally unique identifier of type ModelWithProperties.
static constexpr ::Smp::Uuid Uuid_ModelWithProperties{
    0x3264ed26U, 0x0085U, 0x4238U, 0x8ecfU, 0xd099715341e2U};

class ModelWithPropertiesGen : public ::Xsmp::Model {

  friend class ::Xsmp::Tests::ModelWithProperties;

public:
  // ------------------------------------------------------------------------------------
  // -------------------------- Constructors/Destructor
  // --------------------------
  // ------------------------------------------------------------------------------------

  /// Constructor setting name, description, parent and simulator.
  /// @param name Name of new model instance.
  /// @param description Description of new model instance.
  /// @param parent Parent of new model instance.
  /// @param simulator The simulator instance.
  ModelWithPropertiesGen(::Smp::String8 name, ::Smp::String8 description,
                         ::Smp::IComposite *parent,
                         ::Smp::ISimulator *simulator);
  /// deleted copy constructor
  ModelWithPropertiesGen(const ModelWithPropertiesGen &) = delete;
  /// deleted move constructor
  ModelWithPropertiesGen(ModelWithPropertiesGen &&) = delete;
  /// deleted copy assignment
  ModelWithPropertiesGen &operator=(const ModelWithPropertiesGen &) = delete;
  /// deleted move assignment
  ModelWithPropertiesGen &operator=(ModelWithPropertiesGen &&) = delete;

  /// Virtual destructor to release memory.
  ~ModelWithPropertiesGen() override = default;

  // ----------------------------------------------------------------------------------
  // -------------------------------- IComponent
  // ---------------------------------
  // ----------------------------------------------------------------------------------

  /// Publish fields, operations and properties of the model.
  /// @param receiver Publication receiver.
  void Publish(::Smp::IPublication *receiver) override;

  /// Request for configuration.
  /// @param logger Logger to use for log messages during Configure().
  /// @param linkRegistry Link Registry to use for registration of
  ///         links created during Configure() or later.
  void Configure(::Smp::Services::ILogger *logger,
                 ::Smp::Services::ILinkRegistry *linkRegistry) override;

  /// Connect model to simulator.
  /// @param simulator Simulation Environment that hosts the model.
  ///
  void Connect(::Smp::ISimulator *simulator) override;

  /// Disconnect model to simulator.
  /// @throws Smp::InvalidComponentState
  void Disconnect() override;

  /// Return the Universally Unique Identifier of the Model.
  /// @return Universally Unique Identifier of the Model.
  const Smp::Uuid &GetUuid() const override;

  // ----------------------------------------------------------------------------------
  // --------------------------- IDynamicInvocation ---------------------------
  // ----------------------------------------------------------------------------------
  using RequestHandlers =
      std::map<std::string, std::function<void(ModelWithPropertiesGen *,
                                               ::Smp::IRequest *)>>;
  static RequestHandlers requestHandlers;
  static RequestHandlers InitRequestHandlers();

  /// Invoke the operation for the given request.
  /// @param request Request object to invoke.
  void Invoke(::Smp::IRequest *request) override;

private:
  /// Get enum_property.
  /// @return Current value of property enum_property.
  virtual ::Xsmp::Tests::Types::Enum1 get_enum_property();
  /// Set enum_property.
  /// @param value New value of property enum_property to set.
  virtual void set_enum_property(::Xsmp::Tests::Types::Enum1 value);
  /// Get string_property.
  /// @return Current value of property string_property.
  virtual ::Xsmp::Tests::Types::String1 get_string_property();
  /// Set string_property.
  /// @param value New value of property string_property to set.
  virtual void set_string_property(::Xsmp::Tests::Types::String1 value);
  /// Get int_property.
  /// @return Current value of property int_property.
  virtual ::Smp::Int32 get_int_property();
  /// Set int_property.
  /// @param value New value of property int_property to set.
  virtual void set_int_property(::Smp::Int32 value);
  /// Get float_property.
  /// @return Current value of property float_property.
  virtual ::Smp::Float64 get_float_property();
  /// Set float_property.
  /// @param value New value of property float_property to set.
  virtual void set_float_property(::Smp::Float64 value);
  /// Get readonly_enum_property.
  /// @return Current value of property readonly_enum_property.
  virtual ::Xsmp::Tests::Types::Enum1 get_readonly_enum_property();
  /// Get readonly_string_property.
  /// @return Current value of property readonly_string_property.
  virtual ::Xsmp::Tests::Types::String1 get_readonly_string_property();
  /// Get readonly_int_property.
  /// @return Current value of property readonly_int_property.
  virtual ::Smp::Int32 get_readonly_int_property();
  /// Get readonly_float_property.
  /// @return Current value of property readonly_float_property.
  virtual ::Smp::Float64 get_readonly_float_property();
  /// Set writeonly_enum_property.
  /// @param value New value of property writeonly_enum_property to set.
  virtual void set_writeonly_enum_property(::Xsmp::Tests::Types::Enum1 value);
  /// Set writeonly_string_property.
  /// @param value New value of property writeonly_string_property to set.
  virtual void
  set_writeonly_string_property(::Xsmp::Tests::Types::String1 value);
  /// Set writeonly_int_property.
  /// @param value New value of property writeonly_int_property to set.
  virtual void set_writeonly_int_property(::Smp::Int32 value);
  /// Set writeonly_float_property.
  /// @param value New value of property writeonly_float_property to set.
  virtual void set_writeonly_float_property(::Smp::Float64 value);
  /// Get static_enum_property.
  /// @return Current value of property static_enum_property.
  static ::Xsmp::Tests::Types::Enum1 get_static_enum_property();
  /// Set static_enum_property.
  /// @param value New value of property static_enum_property to set.
  static void set_static_enum_property(::Xsmp::Tests::Types::Enum1 value);
  /// Get static_string_property.
  /// @return Current value of property static_string_property.
  static ::Xsmp::Tests::Types::String1 get_static_string_property();
  /// Set static_string_property.
  /// @param value New value of property static_string_property to set.
  static void set_static_string_property(::Xsmp::Tests::Types::String1 value);
  /// Get static_int_property.
  /// @return Current value of property static_int_property.
  static ::Smp::Int32 get_static_int_property();
  /// Set static_int_property.
  /// @param value New value of property static_int_property to set.
  static void set_static_int_property(::Smp::Int32 value);
  /// Get static_float_property.
  /// @return Current value of property static_float_property.
  static ::Smp::Float64 get_static_float_property();
  /// Set static_float_property.
  /// @param value New value of property static_float_property to set.
  static void set_static_float_property(::Smp::Float64 value);
  /// Get readonly_static_enum_property.
  /// @return Current value of property readonly_static_enum_property.
  static ::Xsmp::Tests::Types::Enum1 get_readonly_static_enum_property();
  /// Get readonly_static_string_property.
  /// @return Current value of property readonly_static_string_property.
  static ::Xsmp::Tests::Types::String1 get_readonly_static_string_property();
  /// Get readonly_static_int_property.
  /// @return Current value of property readonly_static_int_property.
  static ::Smp::Int32 get_readonly_static_int_property();
  /// Get readonly_static_float_property.
  /// @return Current value of property readonly_static_float_property.
  static ::Smp::Float64 get_readonly_static_float_property();
  /// Set writeonly_static_enum_property.
  /// @param value New value of property writeonly_static_enum_property to set.
  static void
  set_writeonly_static_enum_property(::Xsmp::Tests::Types::Enum1 value);
  /// Set writeonly_static_string_property.
  /// @param value New value of property writeonly_static_string_property to
  /// set.
  static void
  set_writeonly_static_string_property(::Xsmp::Tests::Types::String1 value);
  /// Set writeonly_static_int_property.
  /// @param value New value of property writeonly_static_int_property to set.
  static void set_writeonly_static_int_property(::Smp::Int32 value);
  /// Set writeonly_static_float_property.
  /// @param value New value of property writeonly_static_float_property to set.
  static void set_writeonly_static_float_property(::Smp::Float64 value);
  /// Get uuid_property.
  /// @return Current value of property uuid_property.
  virtual ::Smp::Uuid get_uuid_property() = 0;
  /// Set uuid_property.
  /// @param value New value of property uuid_property to set.
  virtual void set_uuid_property(::Smp::Uuid value) = 0;
  ::Xsmp::Tests::Types::Enum1 enum_field;
  ::Xsmp::Tests::Types::String1 string_field;
  ::Smp::Int32 int_field;
  ::Smp::Float64 float_field;
  static ::Xsmp::Tests::Types::Enum1 static_enum_field;
  static ::Xsmp::Tests::Types::String1 static_string_field;
  static ::Smp::Int32 static_int_field;
  static ::Smp::Float64 static_float_field;
};
} // namespace Xsmp::Tests

#endif // XSMP_TESTS_MODELWITHPROPERTIESGEN_H_
