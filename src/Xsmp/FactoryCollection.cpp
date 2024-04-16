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

#include <Smp/CollectionIterator.h>
#include <Xsmp/Exception.h>
#include <Xsmp/FactoryCollection.h>
#include <algorithm>

namespace Xsmp {

FactoryCollection::FactoryCollection(::Smp::IObject *parent)
    : _parent{parent} {}
::Smp::String8 FactoryCollection::GetName() const { return "Factories"; }

::Smp::String8 FactoryCollection::GetDescription() const { return ""; }

::Smp::IObject *FactoryCollection::GetParent() const { return _parent; }
::Smp::IFactory *FactoryCollection::at(::Smp::String8 name) const {
  if (!name)
    return nullptr;
  auto it = _factory_map.find(name);
  return (it == _factory_map.end()) ? nullptr : it->second;
}

::Smp::IFactory *FactoryCollection::at(std::size_t index) const {
  return (index < _factories.size() ? _factories[index] : nullptr);
}

::Smp::IFactory *FactoryCollection::at(::Smp::Uuid uuid) const {
  auto it = _uuid_map.find(uuid);
  return (it == _uuid_map.end()) ? nullptr : it->second.get();
}

std::size_t FactoryCollection::size() const { return _factories.size(); }
FactoryCollection::const_iterator FactoryCollection::begin() const {
  return FactoryCollection::const_iterator(*this, 0);
}
FactoryCollection::const_iterator FactoryCollection::end() const {
  return FactoryCollection::const_iterator(*this, _factories.size());
}

// Add an element to the collection
void FactoryCollection::Add(::Smp::IFactory *factory) {

  auto f = std::unique_ptr<::Smp::IFactory>(factory);
  if (auto it = _uuid_map.find(factory->GetUuid()); it != _uuid_map.end())
    ::Xsmp::Exception::throwDuplicateUuid(this, it->second.get(),
                                          factory->GetName());

  _uuid_map.try_emplace(factory->GetUuid(), std::move(f));
  _factory_map.try_emplace(factory->GetName(), factory);
  _factories.push_back(factory);
}

} // namespace Xsmp
