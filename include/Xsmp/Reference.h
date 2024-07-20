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

#ifndef XSMP_REFERENCE_H_
#define XSMP_REFERENCE_H_

#include <Smp/IComponent.h>
#include <Smp/IReference.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Exception.h>
#include <Xsmp/cstring.h>
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string>
#include <string_view>
#include <vector>

/// XSMP standard types and interfaces.
namespace Xsmp {
class Aggregate;
class Component;

/// XSMP implementation details.
namespace detail {

class AbstractReference : public ::Smp::IReference {
public:
  AbstractReference(::Smp::String8 name, ::Smp::String8 description,
                    ::Xsmp::Aggregate *parent, ::Smp::Int64 lower,
                    ::Smp::Int64 upper);
  AbstractReference(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::IObject *parent, ::Smp::Int64 lower,
                    ::Smp::Int64 upper);
  AbstractReference(const AbstractReference &) = delete;
  AbstractReference &operator=(const AbstractReference &) = delete;
  ~AbstractReference() noexcept override = default;
  ::Smp::String8 GetName() const final;
  ::Smp::String8 GetDescription() const final;
  ::Smp::IObject *GetParent() const final;

  /// Query for the collection of all referenced components.
  /// The returned collection may be empty if no components are
  /// referenced.
  /// @return  Collection of referenced components.
  const ::Smp::ComponentCollection *GetComponents() const final;

  /// Query the maximum number of components in the collection.
  /// A return value of -1 indicates that the collection has no upper
  /// limit.
  ///
  /// @remarks This information can be used to check whether another
  ///          component can be added to the collection.
  /// @remarks This is consistent with the use of upper bounds in UML,
  ///          where a value of -1 represents no limit (typically shown
  ///          as *).
  /// @return  Maximum number of components in the collection (-1 =
  ///          unlimited).
  ::Smp::Int64 GetUpper() const final;

  /// Query the minimum number of components in the collection.
  /// @remarks This information can be used to validate a model
  ///          hierarchy. If a collection specifies a Lower value above
  ///          its current Count, then it is not properly configured. An
  ///          external component may use this information to validate
  ///          the configuration before executing it.
  /// @return  Minimum number of components in the collection or 0 when
  ///          no minimum number has been defined.
  ::Smp::Int64 GetLower() const final;

  /// Query for a referenced component by its name.
  /// The returned component may be null if no component with the given
  /// name could be found.
  /// Multiple components with the same name, but with a different parent
  /// (and hence path) may end up in a single reference. In this case,
  /// retrieving a component by name is not save, as any of the
  /// components that match the name may be returned
  /// @param   name Component name.
  /// @return  Referenced component with the given name, or null if no
  ///          referenced component with the given name could be found.
  ///          If multiple components matching the given name argument
  ///          are found, it returns one of the references.
  ::Smp::IComponent *GetComponent(::Smp::String8 name) const override = 0;

  /// Query for a referenced component by its index.
  /// @param   index Component index.
  /// @return  Referenced component with the given index, or nullptr if no
  ///          referenced component with the given index could be found.
  virtual ::Smp::IComponent *GetComponent(size_t index) const noexcept = 0;

protected:
  // Extensions for ILinkingComponent
  /// Asks a Reference to remove all its links to the given target
  /// component.
  /// After this method has been called, the component must not try to
  /// access the given target component anymore.
  /// @param   target Target component to which all links shall be removed.
  virtual void RemoveLinks(const ::Smp::IComponent *target) = 0;

  friend ::Xsmp::Component;

private:
  class Collection final : public ::Smp::ComponentCollection {
  public:
    using const_iterator = typename ::Smp::ComponentCollection::const_iterator;
    using iterator = typename ::Smp::ComponentCollection::iterator;
    explicit Collection(AbstractReference &parent);

    ::Smp::String8 GetName() const override;
    ::Smp::String8 GetDescription() const override;
    ::Smp::IObject *GetParent() const override;
    ::Smp::IComponent *at(::Smp::String8 name) const override;
    ::Smp::IComponent *at(size_t index) const override;
    size_t size() const override;
    const_iterator begin() const override;
    const_iterator end() const override;

  private:
    AbstractReference &_parent;
  };
  ::Xsmp::cstring _name;
  ::Xsmp::cstring _description;
  ::Smp::IObject *_parent;
  Collection _collection;
  ::Smp::Int64 _lower;
  ::Smp::Int64 _upper;
};

} // namespace detail

/// @class Reference
/// XSMP implementation of ::Smp::IReference.
/// @tparam T The type of components that this Reference instance will manage.
template <class T> class Reference final : public detail::AbstractReference {
public:
  // Concept requirements.
  using value_type = typename std::vector<T *>::value_type;
  using pointer = typename std::vector<T *>::pointer;
  using const_pointer = typename std::vector<T *>::const_pointer;
  using reference = typename std::vector<T *>::reference;
  using const_reference = typename std::vector<T *>::const_reference;
  using iterator = typename std::vector<T *>::iterator;
  using const_iterator = typename std::vector<T *>::const_iterator;
  using const_reverse_iterator =
      typename std::vector<T *>::const_reverse_iterator;
  using reverse_iterator = typename std::vector<T *>::reverse_iterator;
  using size_type = typename std::vector<T *>::size_type;
  using difference_type = typename std::vector<T *>::difference_type;

  /// Constructs a new Reference object with the specified parameters.
  /// @param name The name of the Reference object.
  /// @param description The description of the Reference object.
  /// @param parent The parent Aggregate object that this Reference object
  /// belongs to.
  /// @param lower The minimum number of components that can be contained within
  /// this Reference object.
  /// @param upper The maximum number of components that can be contained within
  /// this Reference object.
  Reference(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Aggregate *parent, ::Smp::Int64 lower, ::Smp::Int64 upper)
      : AbstractReference(name, description, parent, lower, upper) {}

  /// Constructs a new Reference object with the specified parameters.
  /// @param name The name of the Reference object.
  /// @param description The description of the Reference object.
  /// @param parent The parent Aggregate object that this Reference object
  /// belongs to.
  /// @param lower The minimum number of components that can be contained within
  /// this Reference object.
  /// @param upper The maximum number of components that can be contained within
  /// this Reference object.
  Reference(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, ::Smp::Int64 lower, ::Smp::Int64 upper)
      : AbstractReference(name, description, parent, lower, upper) {}

  /// Query for a referenced component by its name.
  /// The returned component may be null if no component with the given
  /// name could be found.
  /// Multiple components with the same name, but with a different parent
  /// (and hence path) may end up in a single reference. In this case,
  /// retrieving a component by name is not save, as any of the
  /// components that match the name may be returned
  /// @param   name Component name.
  /// @return  Referenced component with the given name, or null if no
  ///          referenced component with the given name could be found.
  ///          If multiple components matching the given name argument
  ///          are found, it returns one of the references.
  ::Smp::IComponent *GetComponent(::Smp::String8 name) const override {
    if (name) {
      if (auto it = find(name); it != end()) {
        return dynamic_cast<::Smp::IComponent *>(*it);
      }
    }
    return nullptr;
  }

  /// Query for a referenced component by its index.
  /// @param   index Component index.
  /// @return  Referenced component with the given index, or nullptr if no
  ///          referenced component with the given index could be found.
  ::Smp::IComponent *GetComponent(size_t index) const noexcept override {
    if (index < size()) {
      return dynamic_cast<::Smp::IComponent *>((*this)[index]);
    }
    return nullptr;
  }

  /// Add a referenced component.
  /// This method raises an exception of type ReferenceFull if called for
  /// a full reference, i.e. when the Count has reached the Upper limit.
  /// This method may raise an exception of type InvalidObjectType when
  /// it expects the given component to implement another interface as
  /// well.
  /// Adding a component with a name that already exists in the reference
  /// does not throw an exception, although GetComponent() will no longer
  /// allow to return both referenced components by name.
  /// @param   component New referenced component.
  /// @throws  Smp::InvalidObjectType
  /// @throws  Smp::ReferenceFull
  void AddComponent(::Smp::IComponent *component) override {

    if (size() >= static_cast<size_t>(GetUpper())) {
      ::Xsmp::Exception::throwReferenceFull(this, this->GetCount());
    }
    // check that the component type can be casted to T
    auto *casted = dynamic_cast<T *>(component);
    if (!casted) {
      ::Xsmp::Exception::throwInvalidObjectType<T>(this, component);
    }
    _vector.emplace_back(casted);
  }

  /// Remove a referenced component.
  /// This method raises an exception of type NotReferenced if called
  /// with a component that is not referenced. If the number of
  /// referenced components is less than or equal to the Lower limit,
  /// this method raises an exception of type CannotRemove.
  /// @param   component Referenced component to remove.
  /// @throws  Smp::CannotRemove
  /// @throws  Smp::NotReferenced
  void RemoveComponent(::Smp::IComponent *component) override {

    auto it = find(dynamic_cast<T *>(component));
    if (it == end()) {
      ::Xsmp::Exception::throwNotReferenced(this, component);
    }
    if (size() <= static_cast<size_t>(GetLower())) {
      ::Xsmp::Exception::throwCannotRemove(this, component, GetLower());
    }
    _vector.erase(it);
  }

  /// Query for the number of components in the collection.
  /// @return  Current number of components in the collection.
  ::Smp::Int64 GetCount() const override {
    return static_cast<::Smp::Int64>(this->size());
  }

  /// Get the begin iterator
  /// @return Begin iterator
  iterator begin() noexcept { return _vector.begin(); }

  /// Get the begin iterator
  /// @return Begin iterator
  const_iterator begin() const noexcept { return _vector.begin(); }

  /// Get the end iterator
  /// @return End iterator
  iterator end() noexcept { return _vector.end(); }

  /// Get the end iterator
  /// @return End iterator
  const_iterator end() const noexcept { return _vector.end(); }

  /// Get the reverse begin iterator
  /// @return Reverse begin iterator
  reverse_iterator rbegin() noexcept { return _vector.rbegin(); }

  /// Get the reverse begin iterator
  /// @return Reverse begin iterator
  const_reverse_iterator rbegin() const noexcept { return _vector.rbegin(); }

  /// Get the reverse end iterator
  /// @return Reverse end iterator
  reverse_iterator rend() noexcept { return _vector.rend(); }

  /// Get the reverse end iterator
  /// @return Reverse end iterator
  const_reverse_iterator rend() const noexcept { return _vector.rend(); }

  /// Get the begin iterator
  /// @return Begin iterator
  const_iterator cbegin() const noexcept { return _vector.cbegin(); }

  /// Get the end iterator
  /// @return End iterator
  const_iterator cend() const noexcept { return _vector.cend(); }

  /// Get the reverse begin iterator
  /// @return Reverse begin iterator
  const_reverse_iterator crbegin() const noexcept { return _vector.crbegin(); }

  /// Get the reverse end iterator
  /// @return Reverse end iterator
  const_reverse_iterator crend() const noexcept { return _vector.crend(); }

  /// Get the number of objects in the sequence.
  /// @return Number of objects in the sequence.
  size_type size() const noexcept { return _vector.size(); }

  /// Check if the collection is empty.
  /// @return true if the collection is empty, false otherwise.
  bool empty() const noexcept { return _vector.empty(); }

  /// Find an element by name
  /// @param name the element name
  /// @return the iterator
  iterator find(std::string_view name) {
    return std::find_if(_vector.begin(), _vector.end(), [&name](reference it) {
      return name == dynamic_cast<::Smp::IObject *>(it)->GetName();
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
        _vector.cbegin(), _vector.cend(), [&name](const_reference it) {
          return name == dynamic_cast<const ::Smp::IObject *>(it)->GetName();
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

protected:
  void RemoveLinks(const ::Smp::IComponent *target) override {
    const auto *casted = dynamic_cast<const T *>(target);
    if (!casted) {
      return;
    }
    while (true) {
      auto it = find(casted);
      if (it == cend()) {
        break;
      }
      _vector.erase(it);
    }
  }

private:
  std::vector<T *> _vector;
};

} // namespace Xsmp

#endif // XSMP_REFERENCE_H_
