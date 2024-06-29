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

#ifndef XSMP_PUBLICATION_OPERATION_H_
#define XSMP_PUBLICATION_OPERATION_H_

#include <Smp/IOperation.h>
#include <Smp/IParameter.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IPublishOperation.h>
#include <Smp/Publication/ParameterDirectionKind.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Collection.h>
#include <Xsmp/cstring.h>
#include <memory>

namespace Smp::Publication {
class IType;
class ITypeRegistry;
} // namespace Smp::Publication

namespace Xsmp::Publication {
class Publication;
class Operation final : public ::Smp::Publication::IPublishOperation,
                        public ::Smp::IOperation {
public:
  Operation(::Smp::String8 name, ::Smp::String8 description = "",
            ::Smp::IObject *parent = nullptr,
            ::Smp::ViewKind view = ::Smp::ViewKind::VK_None,
            ::Smp::Publication::ITypeRegistry *typeRegistry = nullptr);
  ~Operation() noexcept override = default;
  /// Operation cannot be copied
  Operation(const Operation &) = delete;
  /// Operation cannot be copied
  Operation &operator=(const Operation &) = delete;

  /// Operation cannot be moved
  Operation(Operation &&) = delete;
  /// Operation cannot be moved
  Operation &operator=(const Operation &&) = delete;

  ::Smp::String8 GetName() const override;
  ::Smp::String8 GetDescription() const override;
  ::Smp::IObject *GetParent() const override;
  /// Publish a parameter of an operation with the given name,
  /// description, type and direction.
  /// If a parameter with the same name has already been published,
  /// and exception of type DuplicateName is thrown.
  /// If the name is not a valid name, an exception of type
  /// InvalidObjectName is thrown.
  /// If no type with the given type UUID exists, an exception of
  /// type TypeNotRegistered is thrown.
  /// @param   name Name of parameter.
  /// @param   description Description of parameter.
  /// @param   typeUuid Uuid of parameter type.
  /// @param   direction Direction kind of parameter.
  /// @throws  ::Smp::DuplicateName
  /// @throws  ::Smp::InvalidObjectName
  /// @throws  ::Smp::Publication::TypeNotRegistered
  void PublishParameter(
      ::Smp::String8 name, ::Smp::String8 description, ::Smp::Uuid typeUuid,
      ::Smp::Publication::ParameterDirectionKind direction =
          ::Smp::Publication::ParameterDirectionKind::PDK_In) override;

  /// Provides the collection of parameters that will end-up in the
  /// request object of CreateRequest(), i.e. all parameter that are not
  /// of direction return. This collection is ordered the same way as the
  /// parameters in the request object, and may be empty.
  /// The optional return parameter is returned via GetReturnParameter.
  /// @return  Collection of parameters, in the same order as in the
  ///          request object created by CreateRequest().
  const ::Smp::ParameterCollection *GetParameters() const override;

  /// Return a parameter by name. This works both for parameters in the
  /// collection of GetParameters(), and for the optional return
  /// parameter.
  /// @param   name Parameter name.
  /// @return  Parameter object, or null if no parameter with the given
  ///          name exists.
  ::Smp::IParameter *GetParameter(::Smp::String8 name) const override;

  /// This operation returns the return parameter, or nullptr if no
  /// return parameter exists (for a void operation).
  /// @return  The return parameter of the operation, or nullptr for a
  ///          void operation.
  ::Smp::IParameter *GetReturnParameter() const override;

  /// Provides the view kind of the operation.
  /// @return  View kind of the operation.
  ::Smp::ViewKind GetView() const override;

  /// Return a request object for the operation that describes the
  /// parameters and the return value.
  /// The request object may be null if the operation does not support
  /// dynamic invocation.
  /// @return  Request object for operation, or null if the operation
  ///          does not support dynamic invocation.
  ::Smp::IRequest *CreateRequest() override;

  /// Dynamically invoke the operation using a request object that has
  /// been created and filled with parameter values by the caller.
  /// @remarks The same request object can be used to invoke an operation
  ///          several times.
  ///          This method raises the InvalidOperationName exception if
  ///          the request object passed does not name the operation, or
  ///          if the operation allows no dynamic invocation. When
  ///          calling invoke with a wrong number of parameters, the
  ///          InvalidParameterCount exception is raised. When passing a
  ///          parameter of wrong type, the InvalidParameterType
  ///          exception is raised.
  /// @param   request Request object to invoke.
  /// @throws  ::Smp::InvalidOperationName
  /// @throws  ::Smp::InvalidParameterCount
  /// @throws  ::Smp::InvalidParameterType
  void Invoke(::Smp::IRequest *request) override;

  /// Destroy a request object that has been created with the
  /// CreateRequest() method before.
  /// The request object must not be used anymore after DeleteRequest()
  /// has been called for it.
  /// @param   request Request object to destroy.
  void DeleteRequest(::Smp::IRequest *request) override;

private:
  /// provide access to Update method to Publication class
  friend class ::Xsmp::Publication::Publication;

  /// Update an Operation that was already published.
  /// The parameters(including the return parameter) are reinitialized
  /// @param description the new Operation description
  /// @param view the new Operation view
  void Update(::Smp::String8 description, ::Smp::ViewKind view) noexcept;

  class Parameter final : public ::Smp::IParameter {
  public:
    Parameter(::Smp::String8 name, ::Smp::String8 description,
              ::Smp::IObject *parent, ::Smp::Publication::IType *type,
              ::Smp::Publication::ParameterDirectionKind direction);

    ~Parameter() noexcept override = default;
    /// Parameter cannot be copied
    Parameter(const Parameter &) = delete;
    /// Parameter cannot be copied
    Parameter &operator=(const Parameter &) = delete;

    /// Parameter cannot be moved
    Parameter(Parameter &&) = delete;
    /// Parameter cannot be moved
    Parameter &operator=(const Parameter &&) = delete;

    ::Smp::String8 GetName() const override;
    ::Smp::String8 GetDescription() const override;
    ::Smp::IObject *GetParent() const override;

    /// Provides the type of the parameter.
    /// @return  Type of the parameter.
    ::Smp::Publication::IType *GetType() const override;

    /// Provides the parameter direction kind of the parameter.
    /// @return  Parameter direction kind of the parameter.
    ::Smp::Publication::ParameterDirectionKind GetDirection() const override;

  private:
    ::Xsmp::cstring _name;
    ::Xsmp::cstring _description;
    ::Smp::IObject *_parent;
    ::Smp::Publication::IType *_type;
    ::Smp::Publication::ParameterDirectionKind _direction;
  };
  ::Xsmp::cstring _name;
  ::Xsmp::cstring _description;
  ::Smp::IObject *_parent;
  std::unique_ptr<::Smp::IParameter> _returnParameter{};
  ::Xsmp::ContainingCollection<::Smp::IParameter> _parameters;
  ::Smp::Publication::ITypeRegistry *_typeRegistry;

  ::Smp::ViewKind _view;
};

} // namespace Xsmp::Publication

#endif // XSMP_PUBLICATION_OPERATION_H_
