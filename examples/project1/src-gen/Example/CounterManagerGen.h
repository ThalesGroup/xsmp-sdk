// Copyright 2023-2024 YOUR ORGANIZATION. All rights reserved.
//
// YOUR NOTICE
// -----------------------------------------------------------------------------
// File Name    : CounterManagerGen.h
// Generated by : XsmpSdkGenerator-1.1.4
// -----------------------------------------------------------------------------
/// @file Example/CounterManagerGen.h
// This file is auto-generated, Do not edit otherwise your changes will be lost

#ifndef EXAMPLE_COUNTERMANAGERGEN_H_
#define EXAMPLE_COUNTERMANAGERGEN_H_

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------
// ----------------------------------------------------------------------------

#include <Example/Counter.h>
#include <Smp/IRequest.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Xsmp/Composite.h>
#include <Xsmp/Container.h>
#include <Xsmp/Model.h>
#include <functional>
#include <map>
#include <type_traits>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------
// ----------------------------------------------------------------------------

namespace Example {
// forward declaration of user class
class CounterManager;
/// Universally unique identifier of type CounterManager.
inline constexpr ::Smp::Uuid Uuid_CounterManager{0x4af1a890U, 0x220eU, 0x4af8U,
                                                 0xa0a9U, 0x2c03aadcfc9dU};

class CounterManagerGen : public ::Xsmp::Model,
                          public virtual ::Xsmp::Composite {

  friend class ::Example::CounterManager;

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
  CounterManagerGen(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::IComposite *parent, ::Smp::ISimulator *simulator);
  /// deleted copy constructor
  CounterManagerGen(const CounterManagerGen &) = delete;
  /// deleted move constructor
  CounterManagerGen(CounterManagerGen &&) = delete;
  /// deleted copy assignment
  CounterManagerGen &operator=(const CounterManagerGen &) = delete;
  /// deleted move assignment
  CounterManagerGen &operator=(CounterManagerGen &&) = delete;

  /// Virtual destructor to release memory.
  ~CounterManagerGen() override = default;

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
      std::map<std::string,
               std::function<void(CounterManagerGen *, ::Smp::IRequest *)>>;
  static RequestHandlers requestHandlers;
  static RequestHandlers InitRequestHandlers();

  /// Invoke the operation for the given request.
  /// @param request Request object to invoke.
  void Invoke(::Smp::IRequest *request) override;

private:
  /// Get sum.
  /// sum of all contained counters
  /// @return Current value of property sum.
  virtual ::Smp::Int32 get_sum() = 0;
  /// Get average.
  /// average of all contained counters
  /// @return Current value of property average.
  virtual ::Smp::Float64 get_average() = 0;
  /// Set count.
  /// set all counter value
  /// @param value New value of property count to set.
  virtual void set_count(::Smp::Int32 value) = 0;

public:
  /// list of handled counters
  ::Xsmp::Container<::Example::Counter> counters;
};
} // namespace Example

#endif // EXAMPLE_COUNTERMANAGERGEN_H_
