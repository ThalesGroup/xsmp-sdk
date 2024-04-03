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
// File Name    : XsmpResolverGen.h
// Generated by : XsmpSdkGenerator-1.1.0
// -----------------------------------------------------------------------------
/// @file Xsmp/Services/XsmpResolverGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef XSMP_SERVICES_XSMPRESOLVERGEN_H_
#define XSMP_SERVICES_XSMPRESOLVERGEN_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------
// ----------------------------------------------------------------------------

#include <Smp/ISimulator.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Services/IResolver.h>
#include <Xsmp/Service.h>
#include <type_traits>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------
// ----------------------------------------------------------------------------

namespace Xsmp::Services {
// forward declaration of user class
class XsmpResolver;
/// Universally unique identifier of type XsmpResolver.
static constexpr ::Smp::Uuid Uuid_XsmpResolver{0x0a942ecbU, 0x5cb5U, 0x4adaU,
                                               0xac98U, 0x54469c1446d5U};

class XsmpResolverGen : public ::Xsmp::Service,
                        public virtual ::Smp::Services::IResolver {

  friend class ::Xsmp::Services::XsmpResolver;

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
  XsmpResolverGen(::Smp::String8 name, ::Smp::String8 description,
                  ::Smp::IComposite *parent, ::Smp::ISimulator *simulator);
  /// deleted copy constructor
  XsmpResolverGen(const XsmpResolverGen &) = delete;
  /// deleted move constructor
  XsmpResolverGen(XsmpResolverGen &&) = delete;
  /// deleted copy assignment
  XsmpResolverGen &operator=(const XsmpResolverGen &) = delete;
  /// deleted move assignment
  XsmpResolverGen &operator=(XsmpResolverGen &&) = delete;

  /// Virtual destructor to release memory.
  ~XsmpResolverGen() override = default;

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
};
} // namespace Xsmp::Services

#endif // XSMP_SERVICES_XSMPRESOLVERGEN_H_
