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

#ifndef XSMP_PUBLICATION_REQUEST_H_
#define XSMP_PUBLICATION_REQUEST_H_

#include <Smp/AnySimple.h>
#include <Smp/IRequest.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IType.h>
#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace Smp {
namespace Publication {
class ITypeRegistry;
} // namespace Publication
class IOperation;
} // namespace Smp

namespace Xsmp::Publication {

class Request: public ::Smp::IRequest {

public:
    Request(::Smp::IOperation *operation,
            ::Smp::Publication::ITypeRegistry *typeRegistry);
    ~Request() noexcept override = default;
    /// Request cannot be copied
    Request(const Request&) = delete;
    /// Request cannot be copied
    Request& operator=(const Request&) = delete;

    /// Request cannot be moved
    Request(Request&&) = delete;
    /// Request cannot be moved
    Request& operator=(const Request&&) = delete;

    /// Return the name of the operation that this request is for.
    /// @remarks A request is typically created using the CreateRequest()
    ///          method to dynamically call a specific method of a
    ///          component implementing the IDynamicInvocation interface.
    ///          This method returns the name passed to it, to allow
    ///          finding out which method is actually called on Invoke().
    /// @return  Name of the operation.
    ::Smp::String8 GetOperationName() const override;

    /// Return the number of parameters stored in the request.
    /// This only considers parameters of direction in, out or in/out, but
    /// not of type return.
    /// Parameters can be accessed by their 0-based index. This index
    ///      - must not be negative,
    ///      - must be smaller than the parameter count.
    /// @remarks The GetParameterIndex() method may be used to access
    ///          parameters by name.
    /// @return  Number of parameters in request object.
    ::Smp::Int32 GetParameterCount() const override;

    /// Query for a parameter index by parameter name.
    /// This only considers parameters of direction in, out or in/out, but
    /// not of type return.
    /// The index values are 0-based. An index of -1 indicates a wrong
    /// parameter name.
    /// @param   parameterName Name of parameter.
    /// @return  Index of parameter with the given name, or -1 if no
    ///          parameter with the given name could be found.
    ::Smp::Int32 GetParameterIndex(::Smp::String8 parameterName) const override;

    /// Assign a value to a parameter at a given position.
    /// This only considers parameters of direction in, out or in/out, but
    /// not of type return.
    /// This method raises an exception of type InvalidParameterIndex if
    /// called with an illegal parameter index. If called with an invalid
    /// parameter type, it raises an exception of type InvalidAnyType. If
    /// called with an invalid value for the parameter, it raises an
    /// exception of type InvalidParameterValue.
    /// @param   index Index of parameter (0-based).
    /// @param   value Value of parameter.
    /// @throws  ::Smp::InvalidAnyType
    /// @throws  ::Smp::InvalidParameterIndex
    /// @throws  ::Smp::InvalidParameterValue
    void SetParameterValue(::Smp::Int32 index, ::Smp::AnySimple value) override;

    /// Query a value of a parameter at a given position.
    /// This only considers parameters of direction in, out or in/out, but
    /// not of type return.
    /// This method raises an exception of type InvalidParameterIndex if
    /// called with an illegal parameter index.
    /// @param   index Index of parameter (0-based).
    /// @return  Value of parameter.
    /// @throws  ::Smp::InvalidParameterIndex
    ::Smp::AnySimple GetParameterValue(::Smp::Int32 index) const override;

    /// Assign the return value of the operation.
    /// This method raises an exception of type VoidOperation if called for
    /// a request object of a void operation. If called with an invalid
    /// return type, it raises an exception of type InvalidAnyType. If
    /// called with an invalid value for the return type, this method
    /// raises an exception of type InvalidReturnValue.
    /// @param   value Return value.
    /// @throws  ::Smp::InvalidAnyType
    /// @throws  ::Smp::InvalidReturnValue
    /// @throws  ::Smp::VoidOperation
    void SetReturnValue(::Smp::AnySimple value) override;

    /// Query the return value of the operation.
    /// This method raises an exception of type VoidOperation if called for
    /// a request object of a void operation.
    /// @return  Return value of the operation.
    /// @throws  ::Smp::VoidOperation
    ::Smp::AnySimple GetReturnValue() const override;

    // check whether a value is valid for a given type
    static bool isValid(const ::Smp::IObject *sender,
            const ::Smp::Publication::IType *type,
            const ::Smp::AnySimple &value);
private:
    ::Smp::IOperation *_operation;

    ::Smp::AnySimple _returnValue { };
    std::map<std::string, ::Smp::Int32, std::less<>> _indexes { };
    std::vector<std::pair<::Smp::AnySimple, const ::Smp::Publication::IType*>> _values { };

    void init(const std::string &name, const ::Smp::Publication::IType *type,
            ::Smp::Publication::ITypeRegistry *typeRegistry);
};

} // namespace Xsmp::Publication

#endif // XSMP_PUBLICATION_REQUEST_H_
