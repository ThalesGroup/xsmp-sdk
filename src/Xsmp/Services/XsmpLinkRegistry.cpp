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

  const std::scoped_lock lck{_linksMutex};
  auto it = _links.find(key);
  if (it == _links.end()) {
    _links.try_emplace(key, 1);
    const std::scoped_lock lck2{_targetsMutex};
    _targets.try_emplace(target, "Links", "", this).first->second.Add(source);
  } else {
    ++it->second;
  }
}

::Smp::UInt32
XsmpLinkRegistry::GetLinkCount(const ::Smp::IComponent *source,
                               const ::Smp::IComponent *target) const {

  const std::scoped_lock lck{_linksMutex};
  if (auto it = _links.find(std::make_pair(source, target));
      it != _links.cend()) {
    return it->second;
  }
  return 0;
}

::Smp::Bool XsmpLinkRegistry::RemoveLink(::Smp::IComponent *source,
                                         const ::Smp::IComponent *target) {
  auto key = std::make_pair(source, target);
  std::unique_lock lck{_linksMutex};
  auto it = _links.find(key);
  if (it == _links.end()) {
    return false;
  }
  it->second--;
  if (it->second == 0) {
    _links.erase(it);
    lck.unlock();
    const std::scoped_lock lck2{_targetsMutex};
    auto it2 = _targets.find(target);
    if (it2 != _targets.end()) {
      it2->second.Remove(source);
    }
  }

  return true;
}

const ::Smp::ComponentCollection *
XsmpLinkRegistry::GetLinkSources(const ::Smp::IComponent *target) const {

  const std::scoped_lock lck{_targetsMutex};

  if (auto it = _targets.find(target); it != _targets.end()) {
    return &it->second;
  }
  return &_targets
              .try_emplace(target, "Links", "",
                           const_cast<XsmpLinkRegistry *>(this))
              .first->second;
}

::Smp::Bool XsmpLinkRegistry::CanRemove(const ::Smp::IComponent *target) {

  const std::scoped_lock lck{_targetsMutex};

  if (auto it = _targets.find(target); it != _targets.end()) {
    for (auto *source : it->second) {
      if (!dynamic_cast<::Smp::ILinkingComponent *>(source)) {
        return false;
      }
    }
  }
  return true;
}

void XsmpLinkRegistry::RemoveLinks(const ::Smp::IComponent *target) {
  std::unique_lock lck{_targetsMutex};
  if (auto it = _targets.find(target); it != _targets.end()) {
    for (auto *source : it->second) {
      if (auto *cmp = dynamic_cast<::Smp::ILinkingComponent *>(source)) {
        lck.unlock();
        cmp->RemoveLinks(target);
        lck.lock();
      }
    }
  }
}

} // namespace Xsmp::Services
