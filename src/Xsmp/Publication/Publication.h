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

#ifndef XSMP_PUBLICATION_PUBLICATION_H_
#define XSMP_PUBLICATION_PUBLICATION_H_

#include <Smp/AccessKind.h>
#include <Smp/IField.h>
#include <Smp/IOperation.h>
#include <Smp/IProperty.h>
#include <Smp/IPublication.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Collection.h>

namespace Smp {
class IComponent;
class IRequest;
namespace Publication {
class ITypeRegistry;
} // namespace Publication
} // namespace Smp

namespace Xsmp::Publication {

class Publication : public ::Smp::IPublication {

public:
  Publication(::Smp::IObject *parent,
              ::Smp::Publication::ITypeRegistry *typeRegistry);
  ~Publication() noexcept override = default;
  /// Publication cannot be copied
  Publication(const Publication &) = delete;
  /// Publication cannot be copied
  Publication &operator=(const Publication &) = delete;

  /// Publication cannot be moved
  Publication(Publication &&) = delete;
  /// Publication cannot be moved
  Publication &operator=(const Publication &&) = delete;

  /// Give access to the global type registry.
  /// The type registry is typically a singleton, and must not be null,
  /// to allow use of existing types, and registration of new types.
  /// @return  Reference to global type registry.
  ::Smp::Publication::ITypeRegistry *GetTypeRegistry() const final;

  /// Publish a Char8 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::Char8 *address,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a Bool field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::Bool *address,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a Int8 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::Int8 *address,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a Int16 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::Int16 *address,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a Int32 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::Int32 *address,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a Int64 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::Int64 *address,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a UInt8 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::UInt8 *address,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a UInt16 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::UInt16 *address,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a UInt32 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::UInt32 *address,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a UInt64 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::UInt64 *address,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a Float32 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::Float32 *address,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a Float64 field with the given name, description, address,
  /// view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::Float64 *address,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a field of any type with the given name, description,
  /// address, type, view kind and state, input and output flags.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// If no type with the given type UUID exists, an exception of type
  /// TypeNotRegistered is thrown.
  /// If the type selected with the type UUID is not a valid value type
  /// for publication of fields, an exception of type InvalidFieldType is
  /// thrown.
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   address Field memory address.
  /// @param   typeUuid Uuid of field type (determines the size).
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidFieldType
  /// @throws  ::Smp::InvalidObjectName
  /// @throws  ::Smp::Publication::TypeNotRegistered
  void PublishField(::Smp::String8 name, ::Smp::String8 description,
                    void *address, ::Smp::Uuid typeUuid,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish a field defined internally that implements the IField
  /// interface.
  /// If a field with the same name has been published before, an
  /// exception of type DuplicateName is thrown.
  /// @param   field Field to publish.
  /// @throws  ::Smp::DuplicateName
  void PublishField(::Smp::IField *field) final;

  /// Publish array of simple type.
  /// This method can only be used for arrays of simple type, as each
  /// simple type can be mapped to a primitive type. The memory layout of
  /// the array has to be without any padding, i.e. the array element
  /// with index i (0-based) is assumed to be stored at address +
  /// i*sizeof(primitiveType).
  /// @param   name Field name.
  /// @param   description Field description.
  /// @param   count Size of array.
  /// @param   address Field memory address.
  /// @param   type Array item type.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @param   input True if field is an input field, false otherwise.
  /// @param   output True if field is an output field, false otherwise.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidFieldType
  /// @throws  ::Smp::InvalidObjectName
  void PublishArray(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::Int64 count, void *address,
                    ::Smp::PrimitiveTypeKind type,
                    ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                    ::Smp::Bool state = true, ::Smp::Bool input = false,
                    ::Smp::Bool output = false) final;

  /// Publish top-level node of an array without using the type registry.
  /// This operation can be used, together with subsequent calls to
  /// PublishField, for direct publication of an array.
  /// This method can be used for arrays of any type. Individual array
  /// elements have to be added manually to the returned IPublication
  /// interface, where each array element can (and has to) be published
  /// with its own memory address.
  /// @param   name Array name.
  /// @param   description Array description.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @return  Interface to publish item type against.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  ::Smp::IPublication *
  PublishArray(::Smp::String8 name, ::Smp::String8 description,
               ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
               ::Smp::Bool state = true) final;

  /// Publish top-level node of a structure without using the type
  /// registry.
  /// This operation can be used, together with subsequent calls to
  /// PublishField, for direct publication of a structure.
  /// @param   name Structure name.
  /// @param   description Structure description.
  /// @param   view Show field in model tree.
  /// @param   state Include field in store/restore of simulation state.
  /// @return  Reference to publish structure fields against.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  ::Smp::IPublication *
  PublishStructure(::Smp::String8 name, ::Smp::String8 description,
                   ::Smp::ViewKind view = ::Smp::ViewKind::VK_All,
                   ::Smp::Bool state = true) final;

  /// Publish an operation with the given name, description and view
  /// kind.
  /// If an Operations with the same name has already been published,
  /// this will update the "description" and "view"  of the previous
  /// publication and it returns the same IPublishOperation of the
  /// previously published Operation.
  /// If an Operation with the same name has not been published, this
  /// creates a new IPublishOperation instance and returns it.
  /// The operation parameters (including an optional return parameter)
  /// may be published against the returned IPublishOperation interface
  /// after calling PublishOperation().
  /// @param   name Name of operation.
  /// @param   description Description of operation.
  /// @param   view View kind of operation node in simulation tree.
  /// @return  Reference to publish parameters against.
  /// @throws  ::Smp::InvalidObjectName
  ::Smp::Publication::IPublishOperation *
  PublishOperation(::Smp::String8 name, ::Smp::String8 description,
                   ::Smp::ViewKind view = ::Smp::ViewKind::VK_None) final;

  /// Publish a property
  /// @param   name Property name.
  /// @param   description Property description.
  /// @param   typeUuid Uuid of type of property.
  /// @param   accessKind Access kind of Property.
  /// @param   view Show field in model tree.
  /// @throws  ::Smp::InvalidObjectName
  /// @throws  ::Smp::Publication::TypeNotRegistered
  void PublishProperty(::Smp::String8 name, ::Smp::String8 description,
                       ::Smp::Uuid typeUuid, ::Smp::AccessKind accessKind,
                       ::Smp::ViewKind view = ::Smp::ViewKind::VK_None) final;

  /// Get the field of given name.
  ///
  /// This method raises an exception of type InvalidFieldName if called
  /// with a field name for which no corresponding field exists.
  /// This method can be used both for fields of simple types (when it
  /// returns an instance of ISimpleField), and for complex fields (when
  /// it returns IArrayField or IStructureField).
  ///
  /// @remarks For getting access to simple fields of structured or array
  ///          types, this method may be called by specifying a full
  ///          field name, e.g. "MyField.Position[2]" in order to access
  ///          an array item of a structure.
  /// @param   fullName Fully qualified field name (relative to the
  ///          component).
  /// @return  Instance of field for given full name.
  /// @throws  ::Smp::InvalidFieldName
  ::Smp::IField *GetField(::Smp::String8 fullName) const override;

  /// Returns a collection of all fields that have been published.
  /// @return  Collection of the fields (immediate children) of the
  ///          component.
  const ::Smp::FieldCollection *GetFields() const override;

  /// Provides the collection of properties that have been published.
  /// @return  Collection of properties that have been published, which
  ///          may be empty.
  const ::Smp::PropertyCollection *GetProperties() const final;

  /// Provides the collection of operations that have been published.
  /// @return  Collection of operations that have been published, which
  ///          may be empty.
  const ::Smp::OperationCollection *GetOperations() const final;

  /// Create request object.
  /// Returns a request object for the given operation that describes the
  /// parameters and the return value.
  /// Request object may be undefined if no operation with given name
  /// could be found.
  /// @param   operationName Name of operation.
  /// @return  Request object for operation.
  ::Smp::IRequest *CreateRequest(::Smp::String8 operationName) final;

  /// Create request object.
  /// Returns a request object for the given property getter that describes the
  /// parameters and the return value.
  /// Request object may be undefined if no property with given name
  /// could be found.
  /// @param   propertyName Name of property.
  /// @return  Request object for property getter.
  virtual ::Smp::IRequest *CreateGetRequest(::Smp::String8 propertyName);

  /// Create request object.
  /// Returns a request object for the given property setter that describes the
  /// parameters and the return value.
  /// Request object may be undefined if no property with given name
  /// could be found.
  /// @param   propertyName Name of property.
  /// @return  Request object for property setter.
  virtual ::Smp::IRequest *CreateSetRequest(::Smp::String8 propertyName);

  /// Delete request object.
  /// Destroys a request object that has been created with the
  /// CreateRequest() method before.
  /// The request object must not be used anymore after DeleteRequest has
  /// been called for it.
  /// @param   request Request object to delete.
  void DeleteRequest(::Smp::IRequest *request) final;

  /// Call this operation to release all data created during earlier
  /// Publish calls to this instance.
  /// This invalidated all pointers retrieved earlier via GetField(),
  /// GetFields() or CreateRequest().
  void Unpublish() final;

private:
  ::Smp::IObject *_parent;
  ::Smp::Publication::ITypeRegistry *_typeRegistry;
  ::Xsmp::ContainingCollection<::Smp::IField> _fields;
  ::Xsmp::DelegateCollection<::Smp::IField> _allFields;
  ::Xsmp::ContainingCollection<::Smp::IProperty> _properties;
  ::Xsmp::ContainingCollection<::Smp::IOperation> _operations;
};

} // namespace Xsmp::Publication

#endif // XSMP_PUBLICATION_PUBLICATION_H_
