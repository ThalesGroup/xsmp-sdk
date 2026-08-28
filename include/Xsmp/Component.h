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

#ifndef XSMP_COMPONENT_H_
#define XSMP_COMPONENT_H_

#include <Smp/AnySimple.h>
#include <Smp/AnySimpleArray.h>
#include <Smp/ComponentStateKind.h>
#include <Smp/IDynamicInvocation.h>
#include <Smp/IField.h>
#include <Smp/ILinkingComponent.h>
#include <Smp/IOperation.h>
#include <Smp/IProperty.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/cstring.h>
#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

namespace Smp {
class IAggregate;
class ICollectionBase;
class IComposite;
class IEventProvider;
class IPublication;
class ISimulator;
namespace Services {
class ILinkRegistry;
class ILogger;
} // namespace Services
} // namespace Smp

/// XSMP standard types and interfaces.
namespace Xsmp {

struct Request;

/// @class Component
/// XSMP implementation of ::Smp::ILinkingComponent and
/// ::Smp::IDynamicInvocation.
class Component : public virtual ::Smp::ILinkingComponent,
                  public virtual ::Smp::IDynamicInvocation {
public:
  /// Constructs a new Component object with the specified name,
  /// description, parent, and simulator.
  /// @param name The name of the component.
  /// @param description The description of the component.
  /// Defaults to an empty string if not provided.
  /// @param parent The parent of this component. Defaults to nullptr if not
  /// provided.
  /// @param simulator A pointer to the ISimulator object associated with this
  /// component. Defaults to nullptr if not provided.
  explicit Component(::Smp::String8 name, ::Smp::String8 description = "",
                     ::Smp::IComposite *parent = nullptr,
                     ::Smp::ISimulator *simulator = nullptr);

  /// Virtual destructor to release memory.
  ~Component() noexcept override = default;

  /// Return the name of the object.
  /// @return  Name of object.
  ::Smp::String8 GetName() const final;

  /// Return the description of the component.
  /// Descriptions are optional and may be empty.
  /// @return  Description of component.
  ::Smp::String8 GetDescription() const final;

  /// Returns the parent object of the component.
  /// @return  Parent object of component or nullptr if component has no parent.
  ::Smp::IObject *GetParent() const final;

  /// Returns the state the component is currently in.
  /// The component state can be changed using the Publish(), Configure()
  /// and Connect() state transition methods.
  /// @return  Current component state.
  ::Smp::ComponentStateKind GetState() const final;

  /// Get the field of given name.
  ///
  /// This method raises an exception of type InvalidFieldName if called
  /// with a field name for which no corresponding field exists.
  /// This method can be used both for fields of simple types (when it
  /// returns an instance of ISimpleField), and for complex fields (when
  /// it returns IArrayField or IStructureField).
  ///
  /// @remarks For getting access to simple fields of structure or array
  ///          types, this method may be called by specifying a full
  ///          field name, e.g. "MyField.Position[2]" in order to access
  ///          an array item of a structure.
  /// @param   fullName Fully qualified field name (relative to the
  ///          component).
  /// @return  Instance of field for given full name.
  /// @throws  Smp::InvalidFieldName
  ::Smp::IField *GetField(::Smp::String8 fullName) const override;

  /// Returns a collection of all fields of the component.
  /// @return  Collection of the fields (immediate children) of the
  ///          component.
  const ::Smp::FieldCollection *GetFields() const override;

  /// Get the value of a field of simple type.
  /// @param   fullName Fully qualified field name (relative to the
  ///          component).
  /// @return  Value of the field.
  /// @throws  Smp::InvalidFieldName
  ::Smp::AnySimple GetSimpleValue(::Smp::String8 fullName) const override;

  /// Set the value of a field of simple type.
  /// @param   fullName Fully qualified field name (relative to the
  ///          component).
  /// @param   value New value for the field.
  /// @throws  Smp::InvalidFieldName
  /// @throws  Smp::InvalidFieldValue
  void SetSimpleValue(::Smp::String8 fullName, ::Smp::AnySimple value) override;

  /// Get the values of a simple array field.
  /// @param   fullName Fully qualified field name (relative to the
  ///          component).
  /// @param   length Size of the preallocated values array.
  /// @param   values Preallocated array of values to store the result to.
  /// @param   startIndex Start index within the simple array.
  /// @throws  Smp::InvalidArraySize
  /// @throws  Smp::InvalidFieldName
  void GetSimpleArrayValue(::Smp::String8 fullName, ::Smp::UInt64 length,
                           ::Smp::AnySimple *values,
                           ::Smp::UInt64 startIndex = 0) const override;

  /// Set the values of a simple array field.
  /// @param   fullName Fully qualified field name (relative to the
  ///          component).
  /// @param   length Size of the values array.
  /// @param   values Array of values to store in the array field.
  /// @param   startIndex Start index within the simple array.
  /// @throws  Smp::InvalidArraySize
  /// @throws  Smp::InvalidArrayValue
  /// @throws  Smp::InvalidFieldName
  void SetSimpleArrayValue(::Smp::String8 fullName, ::Smp::UInt64 length,
                           ::Smp::AnySimpleArray values,
                           ::Smp::UInt64 startIndex = 0) override;

  /// Register a new child object under the component.
  /// Called by the collections of the component that take part in name
  /// resolution, before an object is inserted.
  /// @param   child The child being added to the collection.
  /// @param   collection The collection the object is inserted in.
  /// @return  True if the child was registered, false if its name is
  ///          already taken.
  ::Smp::Bool AddChild(::Smp::IObject *child,
                       const ::Smp::ICollectionBase *collection) override;

  /// Unregister a child object from the component.
  /// @param   child The child being removed from the collection.
  /// @param   collection The collection the object is removed from. It must
  ///          be the one it was registered with.
  /// @return  True if the child was unregistered, false otherwise.
  ::Smp::Bool RemoveChild(::Smp::IObject *child,
                          const ::Smp::ICollectionBase *collection) override;

  /// Look a name up among the children registered for a given collection.
  /// @param   child The name of the child being requested.
  /// @param   collection The collection the object was inserted in.
  /// @return  The child object, or null if that name is not registered for
  ///          that collection.
  ::Smp::IObject *
  IsChildInCollection(::Smp::String8 child,
                      const ::Smp::ICollectionBase *collection) const override;

  /// Returns the child object with the given name.
  /// The children registered through AddChild() are looked up first, then
  /// the published fields.
  /// @param   name The name of the child to look for.
  /// @return  The child with that name, or null if there is no such child.
  ::Smp::IObject *GetChild(::Smp::String8 name) const override;

  /// Request the component to publish its fields, properties and
  /// operations against the provided publication receiver.
  /// This method can only be called once for each component, and only
  /// when the component is in the Created state. The method raises an
  /// InvalidComponentlState exception if the component is not in Created
  /// state.
  /// When this operation is called, the component immediately enters the
  /// Publishing state, before it publishes any of its features.
  /// @remarks The simulation environment typically calls this method in
  ///          the Building state.
  /// @param   receiver Publication receiver.
  /// @throws  Smp::InvalidComponentState
  void Publish(::Smp::IPublication *receiver) override;

  /// Request the component to perform any custom configuration. The
  /// component can create and configure other components using the field
  /// values of its published fields.
  /// This method can only be called once for each component, and only
  /// when the component is in Publishing state. The method raises an
  /// InvalidComponentState exception if the component is not in
  /// Publishing state.
  /// The component can still publish further features in this call, and
  /// can even create other components, but at the end of this call, it
  /// needs to enter the Configured state.
  /// @remarks The simulation environment typically calls this method in
  ///          the Building state.
  /// @param   logger Logger service for logging of error messages during
  ///          configuration.
  /// @param   linkRegistry Reference to the link registry services, so
  ///          that the component can register links that it creates
  ///          during configuration.
  /// @throws  Smp::InvalidComponentState
  void
  Configure(::Smp::Services::ILogger *logger,
            ::Smp::Services::ILinkRegistry *linkRegistry = nullptr) override;

  /// Allow the component to connect to the simulator and its simulation
  /// services.
  /// This method can only be called once for each component, and only
  /// when the component is in the Configured state. The method raises an
  /// InvalidComponentState exception if the component is not in
  /// Configured state.
  /// When this operation is called, the component immediately enters the
  /// Connected state, before it uses any of the simulator methods and
  /// services.
  /// In this method, the component may query for and use any of the
  /// available simulation services, as they are all guaranteed to be
  /// fully functional at that time. It may as well connect to
  /// components' functionality (e.g. to event sources), as it is
  /// guaranteed that all models have been created and configured before
  /// the Connect() method of any component is called.
  /// @remarks The simulation environment typically calls this method in
  ///          the Connecting state.
  /// @param   simulator Simulation Environment that hosts the component.
  /// @throws  Smp::InvalidComponentState
  void Connect(::Smp::ISimulator *simulator) override;

  /// Ask the component to disconnect from the simulator and all its
  /// simulation services.
  /// @throws  Smp::InvalidComponentState
  void Disconnect() override;

  /// Return a request object for the given operation that describes the
  /// parameters and the return value.
  /// The request object may be null if no operation with the given name
  /// could be found, or if the operation with the given name does not
  /// support dynamic invocation.
  /// @param   operationName Name of operation.
  /// @return  Request object for operation, or null if either no
  ///          operation with the given name could be found, or the
  ///          operation with the given name does not support dynamic
  ///          invocation.
  ::Smp::IRequest *CreateRequest(::Smp::String8 operationName);

  /// Dynamically invoke an operation using a request object that has
  /// been created and filled with parameter values by the caller.
  /// @remarks The same request object can be used to invoke a method
  ///          several times.
  ///          This method raises the InvalidOperationName exception if
  ///          the request object passed does not name an operation that
  ///          allows dynamic invocation. When calling invoke with a
  ///          wrong number of parameters, the InvalidParameterCount
  ///          exception is raised. When passing a parameter of wrong
  ///          type, the InvalidParameterValue exception is raised.
  /// @param   request Request object to invoke.
  /// @throws  Smp::InvalidOperationName
  /// @throws  Smp::InvalidParameterCount
  /// @throws  Smp::InvalidParameterValue
  [[noreturn]] void Invoke(::Smp::IRequest *request) override;

  /// Destroy a request object that has been created with the
  /// CreateRequest() method before.
  /// The request object must not be used anymore after DeleteRequest()
  /// has been called for it.
  /// @param   request Request object to destroy.
  void DeleteRequest(::Smp::IRequest *request);

  /// Get a published property by name.
  /// @param   name Name of the property.
  /// @return  The property, or null if there is no such property.
  ::Smp::IProperty *GetProperty(::Smp::String8 name) const override;

  /// Get a published operation by name.
  /// @param   name Name of the operation.
  /// @return  The operation, or null if there is no such operation.
  ::Smp::IOperation *GetOperation(::Smp::String8 name) const override;

  /// Provides the collection of properties that have been published.
  /// @return  Collection of properties that have been published, which
  ///          may be empty.
  const ::Smp::PropertyCollection *GetProperties() const override;

  /// Provides the collection of operations that have been published.
  /// @return  Collection of operations that have been published, which
  ///          may be empty.
  const ::Smp::OperationCollection *GetOperations() const override;

  /// Asks a component to remove all its links to the given target
  /// component.
  /// After this method has been called, the component must not try to
  /// access the given target component anymore.
  /// @param   target Target component to which all links shall be removed.
  void RemoveLinks(const ::Smp::IComponent *target) override;

  /// Get Universally Unique Identifier of Component Type.
  ///
  ///
  /// Operation to get the Universally Unique Identifier of a Component
  /// Type. This allows retrieving metadata from a Catalogue at run-time.
  /// @return  Universally Unique Identifier of Component.
  const ::Smp::Uuid &GetUuid() const override;

protected:
  /// Get the ISimulator object that was provided during construction of this
  /// Component object or associated using the Connect() function.
  [[nodiscard]] inline ::Smp::ISimulator *GetSimulator() const noexcept {
    return _simulator;
  }

  /// Helper class that provides functionalities to call user-defined methods in
  /// derived classes.
  class Helper;

  /// Builds the temporary fields of the operation parameters published by
  /// uuid, which requires the type registry of the simulator.
  friend struct ::Xsmp::Request;

private:
  static void
  RemoveEventProviderLinks(const ::Smp::IEventProvider *eventProvider,
                           const ::Smp::IComponent *target) noexcept;
  static void RemoveAggregateLinks(const ::Smp::IAggregate *aggregate,
                                   const ::Smp::IComponent *target) noexcept;
  static void RemoveFieldLinks(::Smp::IField *field,
                               const ::Smp::IComponent *target) noexcept;

  ::Xsmp::cstring _name;
  ::Xsmp::cstring _description;
  ::Smp::IComposite *_parent;
  ::Smp::ISimulator *_simulator;
  ::Smp::IPublication *_publication = nullptr;
  /// Names registered through AddChild(), with the collections they belong
  /// to. One object can legitimately sit in two collections of the same
  /// component -- a failure is both an ::Smp::IFailure and a published field
  /// -- so a name maps to one object and to every collection that registered
  /// it.
  std::unordered_map<
      std::string,
      std::pair<::Smp::IObject *, std::vector<const ::Smp::ICollectionBase *>>>
      _children;
  ::Smp::ComponentStateKind _state = ::Smp::ComponentStateKind::CSK_Created;
};

} // namespace Xsmp
#endif // XSMP_COMPONENT_H_
