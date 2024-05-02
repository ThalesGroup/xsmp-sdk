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

#ifndef XSMP_CONTAINER_H_
#define XSMP_CONTAINER_H_

#include <Smp/IComponent.h>
#include <Smp/IContainer.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Exception.h>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

namespace Xsmp {
class Composite;
namespace detail {

class AbstractContainer : public ::Smp::IContainer {
public:
  AbstractContainer(::Smp::String8 name, ::Smp::String8 description,
                    ::Xsmp::Composite *parent, ::Smp::Int64 lower,
                    ::Smp::Int64 upper);
  AbstractContainer(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::IObject *parent, ::Smp::Int64 lower,
                    ::Smp::Int64 upper);
  AbstractContainer(const AbstractContainer &) = delete;
  AbstractContainer &operator=(const AbstractContainer &) = delete;
  ~AbstractContainer() noexcept override = default;
  ::Smp::String8 GetName() const final;
  ::Smp::String8 GetDescription() const final;
  ::Smp::IObject *GetParent() const final;

  const ::Smp::ComponentCollection *GetComponents() const final;
  ::Smp::Int64 GetUpper() const final;
  ::Smp::Int64 GetLower() const final;
  ::Smp::IComponent *GetComponent(::Smp::String8 name) const override = 0;
  virtual ::Smp::IComponent *GetComponent(std::size_t index) const = 0;

  void AddComponent(::Smp::IComponent *component) override;
  void DeleteComponent(::Smp::IComponent *component) override;

private:
  class Collection final : public ::Smp::ComponentCollection {
  public:
    using const_iterator = typename ::Smp::ComponentCollection::const_iterator;
    using iterator = typename ::Smp::ComponentCollection::iterator;
    explicit Collection(AbstractContainer &parent);

    ::Smp::String8 GetName() const override;
    ::Smp::String8 GetDescription() const override;
    ::Smp::IObject *GetParent() const override;
    ::Smp::IComponent *at(::Smp::String8 name) const override;
    ::Smp::IComponent *at(std::size_t index) const override;
    std::size_t size() const override;
    const_iterator begin() const override;
    const_iterator end() const override;

  private:
    AbstractContainer &_parent;
  };
  std::string _name;
  std::string _description;
  ::Smp::IObject *_parent;
  Collection _collection;
  ::Smp::Int64 _lower;
  ::Smp::Int64 _upper;
};

} // namespace detail

template <class T>
class Container final : public ::Xsmp::detail::AbstractContainer {
  using holder = std::vector<T *>;

public:
  // Concept requirements.
  using value_type = typename holder::value_type;
  using pointer = typename holder::pointer;
  using const_pointer = typename holder::const_pointer;
  using reference = typename holder::reference;
  using const_reference = typename holder::const_reference;
  using iterator = typename holder::iterator;
  using const_iterator = typename holder::const_iterator;
  using const_reverse_iterator = typename holder::const_reverse_iterator;
  using reverse_iterator = typename holder::reverse_iterator;
  using size_type = typename holder::size_type;
  using difference_type = typename holder::difference_type;

  Container(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Composite *parent, ::Smp::Int64 lower, ::Smp::Int64 upper)
      : AbstractContainer(name, description, parent, lower, upper) {}
  Container(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, ::Smp::Int64 lower, ::Smp::Int64 upper)
      : AbstractContainer(name, description, parent, lower, upper) {}
  Container(const Container &) = delete;
  Container &operator=(const Container &) = delete;

  ~Container() noexcept override {
    // Clear the vector in reverse order
    while (!_vector.empty()) {
      delete _vector.back();
      _vector.pop_back();
    }
  }

  ::Smp::IComponent *GetComponent(::Smp::String8 name) const override {
    if (name) {
      if (auto it = find(name); it != end()) {
        return dynamic_cast<::Smp::IComponent *>(*it);
      }
    }
    return nullptr;
  }

  ::Smp::IComponent *GetComponent(std::size_t index) const override {
    if (index < size()) {
      return dynamic_cast<::Smp::IComponent *>(_vector[index]);
    }
    return nullptr;
  }

  void AddComponent(::Smp::IComponent *component) override {
    AbstractContainer::AddComponent(component);
    // check that the component type can be casted to T
    if (auto *casted = dynamic_cast<T *>(component)) {
      _vector.emplace_back(casted);
    } else {
      ::Xsmp::Exception::throwInvalidObjectType<T>(this, component);
    }
  }
  void DeleteComponent(::Smp::IComponent *component) override {
    if (auto it = find(dynamic_cast<T *>(component)); it != end()) {
      AbstractContainer::DeleteComponent(component);
      _vector.erase(it);
    } else {
      ::Xsmp::Exception::throwNotContained(this, component);
    }
  }
  ::Smp::Int64 GetCount() const override {
    return static_cast<::Smp::Int64>(size());
  }

  // iterators
  iterator begin() noexcept { return _vector.begin(); }
  const_iterator begin() const noexcept { return _vector.begin(); }
  iterator end() noexcept { return _vector.end(); }
  const_iterator end() const noexcept { return _vector.end(); }
  reverse_iterator rbegin() noexcept { return _vector.rbegin(); }
  const_reverse_iterator rbegin() const noexcept { return _vector.rbegin(); }
  reverse_iterator rend() noexcept { return _vector.rend(); }
  const_reverse_iterator rend() const noexcept { return _vector.rend(); }
  const_iterator cbegin() const noexcept { return _vector.cbegin(); }
  const_iterator cend() const noexcept { return _vector.cend(); }
  const_reverse_iterator crbegin() const noexcept { return _vector.crbegin(); }
  const_reverse_iterator crend() const noexcept { return _vector.crend(); }

  // capacity
  size_type size() const noexcept { return _vector.size(); }
  bool empty() const noexcept { return _vector.empty(); }

  // element access
  /// Find an element by name
  /// @param name the element name
  /// @return the iterator
  iterator find(std::string_view name) {
    return std::find_if(_vector.begin(), _vector.end(), [name](reference it) {
      return name == dynamic_cast<::Smp::IComponent *>(it)->GetName();
    });
  }
  /// Find an element
  /// @param elem the element to find
  /// @return the iterator
  iterator find(T *elem) {
    return std::find(_vector.begin(), _vector.end(), elem);
  }
  /// Find an element by name
  /// @param name the element name
  /// @return the iterator
  const_iterator find(std::string_view name) const {
    return std::find_if(
        _vector.begin(), _vector.end(), [name](const_reference it) {
          return name == dynamic_cast<const ::Smp::IComponent *>(it)->GetName();
        });
  }
  /// Find an element
  /// @param elem the element to find
  /// @return the iterator
  iterator find(const T *elem) {
    return std::find(_vector.begin(), _vector.end(), elem);
  }
  /// Get an element by index
  /// @param index the index
  /// @return the element or undefined value if index is out of range
  reference operator[](size_type index) { return _vector[index]; }
  /// Get an element by name
  /// @param name the element name
  /// @return the element or undefined value if name is not found
  reference operator[](std::string_view name) { return *find(name); }
  /// Get an element by index
  /// @param index the index
  /// @return the element or undefined value if index is out of range
  const_reference operator[](size_type index) const { return _vector[index]; }
  /// Get an element by name
  /// @param name the element name
  /// @return the element or undefined value if name is not found
  const_reference operator[](std::string_view name) const {
    return *find(name);
  }
  /// Get an element by index
  /// @param index the index
  /// @return the element
  /// @throws std::out_of_range if the element is not found
  reference at(size_type index) { return _vector.at(index); }
  /// Get an element by name
  /// @param name the element name
  /// @return the element
  /// @throws std::out_of_range if the element is not found
  reference at(std::string_view name) {
    if (auto it = find(name); it != _vector.end()) {
      return *it;
    }
    throw std::out_of_range("No Object named: " + std::string(name));
  }
  /// Get an element by index
  /// @param index the index
  /// @return the element
  /// @throws std::out_of_range if the element is not found
  const_reference at(size_type index) const { return _vector.at(index); }
  /// Get an element by name
  /// @param name the element name
  /// @return the element
  /// @throws std::out_of_range if the element is not found
  const_reference at(std::string_view name) const {
    if (auto it = find(name); it != _vector.cend()) {
      return *it;
    }
    throw std::out_of_range("No Object named: " + std::string(name));
  }
  /// Get the first element
  /// @return the first element
  reference front() { return _vector.front(); }
  /// Get the first element
  /// @return the first element
  const_reference front() const { return _vector.front(); }
  /// Get the last element
  /// @return the last element
  reference back() { return _vector.back(); }
  /// Get the last element
  /// @return the last element
  const_reference back() const { return _vector.back(); }

private:
  holder _vector{};
};

} // namespace Xsmp

#endif // XSMP_CONTAINER_H_
