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

#ifndef XSMP_COMPOSITE_H_
#define XSMP_COMPOSITE_H_

#include <Smp/IComposite.h>
#include <Smp/IContainer.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Collection.h>

/// XSMP standard types and interfaces.
namespace Xsmp {
/// XSMP implementation details.
namespace detail {
class AbstractContainer;
} // namespace detail

/// @class Composite
/// XSMP implementation of ::Smp::IComposite.
class Composite : public virtual ::Smp::IComposite {
public:
  /// Default constructor.
  Composite();

  /// Virtual destructor to release memory.
  ~Composite() noexcept override = default;

  /// Query for the collection of all containers of the composite.
  /// The returned collection may be empty if no containers exist for the
  /// composite.
  /// @return  Collection of containers.
  const ::Smp::ContainerCollection *GetContainers() const override;

  /// Query for a container of this composite by its name.
  /// The returned container may be null if no container with the given
  /// name could be found.
  /// @param   name Container name.
  /// @return  Container queried for by name, or null if no container
  ///          with this name exists.
  ::Smp::IContainer *GetContainer(::Smp::String8 name) const override;

protected:
  /// Add a container to the collection of managed containers.
  /// @param container Pointer to the ::Smp::IContainer to add.
  inline void AddContainer(::Smp::IContainer *container) {
    _containers.Add(container);
  }

private:
  friend ::Xsmp::detail::AbstractContainer;
  ::Xsmp::Collection<::Smp::IContainer> _containers;
};

} // namespace Xsmp
#endif // XSMP_COMPOSITE_H_
