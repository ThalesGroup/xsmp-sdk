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

#ifndef XSMP_COLLECTION_H_
#define XSMP_COLLECTION_H_

#include <Smp/ICollection.h>
#include <Smp/IObject.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Object.h>
#include <algorithm>
#include <stddef.h>
#include <string_view>
#include <unordered_map>
#include <vector>

/// XSMP standard types and interfaces.
namespace Xsmp {
/// XSMP implementation details.
namespace detail {
template <typename T> class AbstractCollection : public ::Smp::ICollection<T> {
public:
  using const_iterator = typename ::Smp::ICollection<T>::const_iterator;
  using iterator = typename ::Smp::ICollection<T>::iterator;

  /// Retrieve element by name
  /// @param name The name of the element to be retrieved.
  /// @return Instance with given name, or nullptr if no such instance exists.
  T *at(::Smp::String8 name) const override {
    if (!name) {
      return nullptr;
    }
    auto it = _map.find(name);
    return it == _map.cend() ? nullptr : it->second;
  }

  /// Retrieve element by position in the sequence (based on order of
  /// insertion).
  /// @param index The position of the elment to be retrieved.
  /// @return Instance at given index, or nullptr if no instance exists.
  T *at(size_t index) const override {
    return (index < _vector.size() ? _vector[index] : nullptr);
  }

  /// Get the number of objects in the sequence.
  /// @return Number of objects in the sequence.
  size_t size() const override { return _vector.size(); }

  /// Check if the collection is empty.
  /// @return true if the collection is empty, false otherwise.
  virtual bool empty() const { return _vector.empty(); };

  /// Get the begin iterator
  /// @return Begin iterator
  const_iterator begin() const override {
    return AbstractCollection<T>::const_iterator(*this, 0);
  }

  /// Get the end iterator
  /// @return End iterator
  const_iterator end() const override {
    return AbstractCollection<T>::const_iterator(*this, _vector.size());
  }

  /// Add an element to the collection
  /// @param element The element to add to the collection.
  void Add(T *element) {
    const auto *casted = dynamic_cast<const ::Smp::IObject *>(element);
    if (!casted) {
      ::Xsmp::Exception::throwException(
          this, "InvalidObject", "",
          "Tried to add an element that is not an ::Smp::IObject");
    }
    if (this->at(casted->GetName())) {
      ::Xsmp::Exception::throwDuplicateName(this, casted->GetName(), this);
    }
    _map.emplace(casted->GetName(), element);
    _vector.push_back(element);
  }

  /// Remove an element from the collection.
  /// @param element The element to be removed from the collection.
  void Remove(T *element) {
    const auto *casted = dynamic_cast<const ::Smp::IObject *>(element);
    if (!casted) {
      ::Xsmp::Exception::throwException(
          this, "InvalidObject", "",
          "Tried to remove an element that is not an ::Smp::IObject");
    }

    if (auto it = _map.find(casted->GetName()); it != _map.end()) {
      _map.erase(it);
    }
    const auto it2 = std::find(_vector.begin(), _vector.end(), element);
    if (it2 != _vector.cend()) {
      _vector.erase(it2);
    }
  }

  /// Removes all elements from the collection.
  void clear() {
    _vector.clear();
    _map.clear();
  }

private:
  std::vector<T *> _vector;
  std::unordered_map<std::string_view, T *> _map;
};
} // namespace detail

/// @class Collection
/// XSMP implementation of ::Smp::ICollection.
/// The Collection instance does not own the elements it contains.
template <typename T>
class Collection final : public ::Xsmp::Object,
                         public ::Xsmp::detail::AbstractCollection<T> {
public:
  using ::Xsmp::Object::Object;
};

/// @class ContainingCollection
/// XSMP implementation of ::Smp::ICollection.
/// The Collection instance owns the elements it contains.
template <typename T>
class ContainingCollection final : public ::Xsmp::Object,
                                   public ::Smp::ICollection<T> {
public:
  using ::Xsmp::Object::Object;

  using const_iterator = typename ::Smp::ICollection<T>::const_iterator;
  using iterator = typename ::Smp::ICollection<T>::iterator;

  /// Retrieve element by name
  /// @param name The name of the element to be retrieved.
  /// @return Instance with given name, or nullptr if no such instance exists.
  T *at(::Smp::String8 name) const override {
    if (!name) {
      return nullptr;
    }
    auto it = _map.find(name);
    return it == _map.cend() ? nullptr : it->second.get();
  }

  /// Retrieve element by position in the sequence (based on order of
  /// insertion).
  /// @param index The position of the elment to be retrieved.
  /// @return Instance at given index, or nullptr if no instance exists.
  T *at(size_t index) const override {
    return (index < _vector.size() ? _vector[index] : nullptr);
  }

  /// Get the number of objects in the sequence.
  /// @return Number of objects in the sequence.
  size_t size() const override { return _vector.size(); }

  /// Check if the collection is empty.
  /// @return true if the collection is empty, false otherwise.
  virtual bool empty() const { return _vector.empty(); };

  /// Get the begin iterator
  /// @return Begin iterator
  const_iterator begin() const override { return const_iterator(*this, 0); }

  /// Get the end iterator
  /// @return End iterator
  const_iterator end() const override {
    return const_iterator(*this, _vector.size());
  }

  /// Add an element to the collection
  /// @tparam U The type of the element to add
  /// @tparam Args Variable argument types used in constructing the element
  /// @param args Arguments to be forwarded to the constructor of U
  /// @return A pointer to the added element
  template <typename U, class... Args> U *Add(Args &&...args) {
    return Add(std::make_unique<U>(std::forward<Args>(args)...));
  }

  /// Add an element to the collection
  /// @tparam U The type of the element to add
  /// @param element A unique pointer to the element to be added
  /// @return A raw pointer to the added element
  template <typename U> U *Add(std::unique_ptr<U> element) {

    auto *raw = element.get();
    const auto *casted = dynamic_cast<const ::Smp::IObject *>(raw);
    if (!casted) {
      ::Xsmp::Exception::throwException(
          this, "InvalidObject", "",
          "Tried to add an element that is not an ::Smp::IObject");
    }
    if (this->at(casted->GetName())) {
      ::Xsmp::Exception::throwDuplicateName(this, casted->GetName(), this);
    }

    _vector.push_back(raw);
    _map.emplace(casted->GetName(), std::move(element));
    return raw;
  }

  /// Remove an element from the collection.
  /// @param element The element to be removed from the collection.
  void Remove(T *element) {
    const auto *casted = dynamic_cast<const ::Smp::IObject *>(element);
    if (!casted) {
      ::Xsmp::Exception::throwException(
          this, "InvalidObject", "",
          "Tried to remove an element that is not an ::Smp::IObject");
    }

    if (auto it = _map.find(casted->GetName()); it != _map.end()) {
      _map.erase(it);
    }

    if (auto it = std::find(_vector.begin(), _vector.end(), element);
        it != _vector.end()) {
      _vector.erase(it);
    }
  }

  /// Removes all elements from the collection.
  void clear() {
    _vector.clear();
    _map.clear();
  }

private:
  std::vector<T *> _vector;
  std::unordered_map<std::string_view, std::unique_ptr<T>> _map;
};

/// @class DelegateCollection
/// XSMP implementation of ::Smp::ICollection.
/// The Collection instance does not own the elements it contains.
/// The Collection contains its own elements and delegates to another
/// collection for other elements.
template <typename T>
class DelegateCollection final : public ::Xsmp::detail::AbstractCollection<T> {
public:
  using const_iterator = typename ::Smp::ICollection<T>::const_iterator;
  using iterator = typename ::Smp::ICollection<T>::iterator;

  explicit DelegateCollection(const ::Smp::ICollection<T> *delegate)
      : _delegate(delegate) {}
  DelegateCollection(const DelegateCollection &) = delete;
  DelegateCollection &operator=(const DelegateCollection &) = delete;

  /// Retrieve element by name
  /// @param name The name of the element to be retrieved.
  /// @return Instance with given name, or nullptr if no such instance exists.
  T *at(::Smp::String8 name) const override {
    auto *value = _delegate->at(name);
    return value ? value : ::Xsmp::detail::AbstractCollection<T>::at(name);
  }

  /// Retrieve element by position in the sequence (based on order of
  /// insertion).
  /// @param index The position of the elment to be retrieved.
  /// @return Instance at given index, or nullptr if no instance exists.
  T *at(size_t index) const override {
    return index < _delegate->size()
               ? _delegate->at(index)
               : ::Xsmp::detail::AbstractCollection<T>::at(index -
                                                           _delegate->size());
  }

  /// Get the number of objects in the sequence.
  /// @return Number of objects in the sequence.
  size_t size() const override {
    return detail::AbstractCollection<T>::size() + _delegate->size();
  }

  /// Check if the collection is empty.
  /// @return true if the collection is empty, false otherwise.
  bool empty() const override {
    return detail::AbstractCollection<T>::empty() && (_delegate->size() == 0);
  }

  /// Get the begin iterator
  /// @return Begin iterator
  const_iterator begin() const override {
    return DelegateCollection<T>::const_iterator(*this, 0);
  }

  /// Get the end iterator
  /// @return End iterator
  const_iterator end() const override {
    return DelegateCollection<T>::const_iterator(*this, size());
  }

  ::Smp::String8 GetName() const override { return _delegate->GetName(); }
  ::Smp::String8 GetDescription() const override {
    return _delegate->GetDescription();
  }
  ::Smp::IObject *GetParent() const override { return _delegate->GetParent(); }

private:
  const ::Smp::ICollection<T> *_delegate;
};

} // namespace Xsmp

#endif // XSMP_COLLECTION_H_
