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

#ifndef XSMP_SERVICES_XSMPLINKREGISTRY_H_
#define XSMP_SERVICES_XSMPLINKREGISTRY_H_

#include <Smp/IComponent.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Collection.h>
#include <Xsmp/Services/XsmpLinkRegistryGen.h>
#include <map>
#include <mutex>
#include <utility>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------
// ----------------------------------------------------------------------------

namespace Xsmp::Services {
class XsmpLinkRegistry final : public XsmpLinkRegistryGen {
public:
  // ------------------------------------------------------------------------------------
  // -------------------------- Constructors/Destructor
  // --------------------------
  // ------------------------------------------------------------------------------------

  /// Constructor setting name, description and parent.
  using XsmpLinkRegistryGen::XsmpLinkRegistryGen;

  /// Virtual destructor to release memory.
  ~XsmpLinkRegistry() noexcept override = default;

  /// Add a link from source component to target component.
  /// This increments the link count between source and target.
  /// This method informs the link registry that a link between two
  /// components has been created. The link registry does not create
  /// this link, it only gets told about its existence.
  /// This method can be called several times with the same
  /// arguments, when a source component has several links to the
  /// same target component.
  /// @param   source Source component of link (i.e. the component
  ///          that links to another component).
  /// @param   target Target component of link (i.e. the component
  ///          that is being linked to by another component).
  void AddLink(::Smp::IComponent *source,
               const ::Smp::IComponent *target) override;

  /// Returns the number of links between source and target.
  /// This number is incremented each time AddLink() is called, and
  /// decremented each time that RemoveLink() is called.
  /// @param   source Source component of link (i.e. the component
  ///          that links to another component).
  /// @param   target Target component of link (i.e. the component
  ///          that is being linked to by another component).
  /// @return  True if such a link has been added before (and not
  ///          been removed), false otherwise.
  ::Smp::UInt32 GetLinkCount(const ::Smp::IComponent *source,
                             const ::Smp::IComponent *target) const override;

  /// Remove a link between source and target that has been added to
  /// the service using AddLink() before.
  /// This decrements the link count between source and target.
  /// This method informs the link registry that a link between two
  /// components has been removed. The link registry does not remove
  /// this link, it only gets told about the fact that the link no
  /// longer exists.
  /// This method can be called several times with the same
  /// arguments, when a source component had several links to the
  /// same target component.
  /// @param   source Source component of link (i.e. the component
  ///          that links to another component).
  /// @param   target Target component of link (i.e. the component
  ///          that is being linked to by another component).
  /// @return  True if the link count between source and target had
  ///          been positive and has been decremented, false if the
  ///          link count had already been 0.
  ::Smp::Bool RemoveLink(::Smp::IComponent *source,
                         const ::Smp::IComponent *target) override;

  /// Returns a collection of all sources that have a link to the
  /// given target.
  /// This method returns the collection of source components for
  /// which a link to the given target component has been added to
  /// the link registry.
  /// @param   target Target component to return link sources for.
  /// @return  Collection of source components which link to the
  ///          given target.
  const ::Smp::ComponentCollection *
  GetLinkSources(const ::Smp::IComponent *target) const override;

  /// Returns true if all sources linking to the given target can be
  /// asked to remove their link(s), false otherwise.
  /// This method checks whether all sources that have a link to the
  /// given target implement the optional interface
  /// ILinkingComponent. If so, they can be asked to remove their
  /// links. The method returns false if at least one source exists
  /// that does not implement the ILinkingComponent interface.
  /// @param   target Target component to check for links.
  /// @return  True if all links to the given target can be removed,
  ///          false otherwise.
  ::Smp::Bool CanRemove(const ::Smp::IComponent *target) override;

  /// Removes all links to the given target.
  /// This method calls the RemoveLinks() method of all source
  /// components that implement the optional ILinkingComponent
  /// interface, so it asks all link sources to remove their links to
  /// the given target.
  /// @param   target Target component of link (i.e. the component
  ///          that is being linked to by other components).
  void RemoveLinks(const ::Smp::IComponent *target) override;

private:
  friend class ::Xsmp::Component::Helper;
  mutable std::mutex _linksMutex;
  std::map<std::pair<const ::Smp::IComponent *, const ::Smp::IComponent *>,
           ::Smp::UInt32>
      _links;

  mutable std::mutex _targetsMutex;
  mutable std::map<const ::Smp::IComponent *,
                   ::Xsmp::Collection<::Smp::IComponent>>
      _targets;
};
} // namespace Xsmp::Services

#endif // XSMP_SERVICES_XSMPLINKREGISTRY_H_
