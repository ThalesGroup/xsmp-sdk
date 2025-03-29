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

#include <Smp/IComponent.h>
#include <Smp/ILinkingComponent.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Services/XsmpLinkRegistry.h>
#include <mutex>
#include <utility>

namespace Xsmp::Services {

void XsmpLinkRegistry::AddLink(::Smp::IComponent *source,
                               const ::Smp::IComponent *target) {

  auto key = std::make_pair(source, target);

  auto linksAccess = _links.write();
  auto it = linksAccess.get().find(key);
  if (it == linksAccess.get().end()) {
    linksAccess.get().try_emplace(key, 1);
    _targets.write()
        .get()
        .try_emplace(target, "Links", "", this)
        .first->second.Add(source);
  } else {
    ++it->second;
  }
}

::Smp::UInt32
XsmpLinkRegistry::GetLinkCount(const ::Smp::IComponent *source,
                               const ::Smp::IComponent *target) const {
  auto linksAccess = _links.read();
  if (auto it = linksAccess.get().find(std::make_pair(source, target));
      it != linksAccess.get().cend()) {
    return it->second;
  }
  return 0;
}

::Smp::Bool XsmpLinkRegistry::RemoveLink(::Smp::IComponent *source,
                                         const ::Smp::IComponent *target) {
  auto key = std::make_pair(source, target);
  auto linksAccess = _links.write();
  auto it = linksAccess.get().find(key);
  if (it == linksAccess.get().end()) {
    return false;
  }
  it->second--;
  if (it->second == 0) {
    linksAccess.get().erase(it);
    linksAccess.unlock();
    auto targetsAccess = _targets.write();
    auto it2 = targetsAccess.get().find(target);
    if (it2 != targetsAccess.get().end()) {
      it2->second.Remove(source);
    }
  }

  return true;
}

const ::Smp::ComponentCollection *
XsmpLinkRegistry::GetLinkSources(const ::Smp::IComponent *target) const {

  auto targetAccess = _targets.read();

  if (auto it = targetAccess.get().find(target);
      it != targetAccess.get().end()) {
    return &it->second;
  }
  targetAccess.unlock();

  return &_targets.write()
              .get()
              .try_emplace(target, "Links", "",
                           const_cast<XsmpLinkRegistry *>(this))
              .first->second;
}

::Smp::Bool XsmpLinkRegistry::CanRemove(const ::Smp::IComponent *target) {

  auto targetAccess = _targets.read();

  if (auto it = targetAccess.get().find(target);
      it != targetAccess.get().end()) {
    for (auto *source : it->second) {
      if (!dynamic_cast<::Smp::ILinkingComponent *>(source)) {
        return false;
      }
    }
  }
  return true;
}

void XsmpLinkRegistry::RemoveLinks(const ::Smp::IComponent *target) {
  auto targetAccess = _targets.read();
  if (auto it = targetAccess.get().find(target);
      it != targetAccess.get().end()) {
    for (auto *source : it->second) {
      if (auto *cmp = dynamic_cast<::Smp::ILinkingComponent *>(source)) {
        targetAccess.unlock();
        cmp->RemoveLinks(target);
        targetAccess.lock();
      }
    }
  }
}

} // namespace Xsmp::Services
