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

#ifndef XSMP_AGGREGATE_H_
#define XSMP_AGGREGATE_H_

#include <Smp/IAggregate.h>
#include <Smp/IReference.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Collection.h>

/// XSMP standard types and interfaces.
namespace Xsmp {

/// XSMP implementation details.
namespace detail {
class AbstractReference;
} // namespace detail

/// @class Aggregate
/// XSMP implementation of ::Smp::IAggregate.
class Aggregate : public virtual ::Smp::IAggregate {
public:
  /// Default constructor
  Aggregate();

  /// Query for the collection of all references of the aggregate
  /// component.
  /// The returned collection may be empty if no references exist for the
  /// aggregate.
  /// @return  Collection of references.
  const ::Smp::ReferenceCollection *GetReferences() const override;

  /// Query for a reference of this aggregate component by its name.
  /// The returned reference may be null if no reference with the given
  /// name could be found. If more than one reference with this name
  /// exists, it is not defined which one is returned.
  /// @param   name Reference name.
  /// @return  Reference queried for by name, or null if no reference
  ///          with this name exists.
  ::Smp::IReference *GetReference(::Smp::String8 name) const override;

protected:
  /// Adds a reference to this aggregate component.
  /// @param reference Pointer to the reference to add
  inline void AddReference(::Smp::IReference *reference) {
    _references.Add(reference);
  }

private:
  friend ::Xsmp::detail::AbstractReference;
  ::Xsmp::Collection<::Smp::IReference> _references;
};

} // namespace Xsmp
#endif // XSMP_AGGREGATE_H_
