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
#include <Xsmp/Aggregate.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Reference.h>

namespace Xsmp::detail {

AbstractReference::Collection::Collection(AbstractReference &parent)
    : _parent(parent) {}
::Smp::String8 AbstractReference::Collection::GetName() const {
  return "Collection";
}
::Smp::String8 AbstractReference::Collection::GetDescription() const {
  return "Collection of component";
}
::Smp::IObject *AbstractReference::Collection::GetParent() const {
  return &_parent;
}
::Smp::IComponent *
AbstractReference::Collection::at(::Smp::String8 name) const {
  return _parent.GetComponent(name);
}
::Smp::IComponent *AbstractReference::Collection::at(std::size_t index) const {
  return _parent.GetComponent(index);
}
std::size_t AbstractReference::Collection::size() const {
  return static_cast<std::size_t>(_parent.GetCount());
}
AbstractReference::Collection::const_iterator
AbstractReference::Collection::begin() const {
  return const_iterator(*this, 0);
}
AbstractReference::Collection::const_iterator
AbstractReference::Collection::end() const {
  return const_iterator(*this, size());
}

AbstractReference::AbstractReference(::Smp::String8 name,
                                     ::Smp::String8 description,
                                     ::Xsmp::Aggregate *parent,
                                     ::Smp::Int64 lower, ::Smp::Int64 upper)
    : AbstractReference(name, description,
                        static_cast<::Smp::IObject *>(parent), lower, upper) {
  parent->AddReference(this);
}
AbstractReference::AbstractReference(::Smp::String8 name,
                                     ::Smp::String8 description,
                                     ::Smp::IObject *parent, ::Smp::Int64 lower,
                                     ::Smp::Int64 upper)
    : _name(::Xsmp::Helper::checkName(name, parent)),
      _description(description ? description : ""), _parent(parent),
      _collection(*this), _lower(lower), _upper(upper) {}
::Smp::String8 AbstractReference::GetName() const { return _name.c_str(); }

::Smp::String8 AbstractReference::GetDescription() const {
  return _description.c_str();
}

::Smp::IObject *AbstractReference::GetParent() const { return _parent; }
const ::Smp::ComponentCollection *AbstractReference::GetComponents() const {
  return &_collection;
}
::Smp::Int64 AbstractReference::GetUpper() const { return _upper; }
::Smp::Int64 AbstractReference::GetLower() const { return _lower; }

} // namespace Xsmp::detail
