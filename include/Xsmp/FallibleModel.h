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

#ifndef XSMP_FALLIBLEMODEL_H_
#define XSMP_FALLIBLEMODEL_H_

#include <Smp/IFailure.h>
#include <Smp/IFallibleModel.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Collection.h>

/// XSMP standard types and interfaces.
namespace Xsmp {

/// XSMP implementation details.
namespace detail {
class Failure;
} // namespace detail

/// @class FallibleModel
/// XSMP implementation of ::Smp::IFallibleModel.
class FallibleModel : public virtual ::Smp::IFallibleModel {
public:
  /// Default constructor
  FallibleModel();

  /// Query for whether the model is failed. A model is failed when at
  /// least one of its failures is failed.
  /// @return  Whether the model is failed or not.
  ::Smp::Bool IsFailed() const override;

  /// Query for the collection of all failures.
  /// The returned collection may be empty if no failures exist for the
  /// fallible model.
  /// @return  Failure collection of the model.
  const ::Smp::FailureCollection *GetFailures() const override;

  /// Get a failure by name.
  /// The returned failure may be null if no child with the given name
  /// could be found.
  /// @param   name Name of the failure to return.
  /// @return  Failure queried for by name, or null if no failure with
  ///          this name exists.
  ::Smp::IFailure *GetFailure(::Smp::String8 name) const override;

protected:
  /// Adds a failure to this FallibleModel.
  /// @param failure Pointer to the failure to add
  inline void AddFailure(::Smp::IFailure *failure) { _failures.Add(failure); }
  friend class ::Xsmp::detail::Failure;

private:
  ::Xsmp::Collection<::Smp::IFailure> _failures;
};

} // namespace Xsmp

#endif // XSMP_FALLIBLEMODEL_H_
