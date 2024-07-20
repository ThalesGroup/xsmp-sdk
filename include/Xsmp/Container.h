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
#include <Xsmp/cstring.h>
#include <algorithm>
#include <cstddef>
#include <stdexcept>
#include <string_view>
#include <vector>

/// XSMP standard types and interfaces.
namespace Xsmp {
class Composite;

/// XSMP implementation details.
namespace detail {

/// @class AbstractContainer
/// XSMP implementation of ::Smp::IContainer.
class AbstractContainer : public ::Smp::IContainer {
public:
  AbstractContainer(::Smp::String8 name, ::Smp::String8 description,
                    ::Xsmp::Composite *parent, ::Smp::Int64 lower,
                    ::Smp::Int64 upper);

  AbstractContainer(::Smp::String8 name, ::Smp::String8 description,
                    ::Smp::IObject *parent, ::Smp::Int64 lower,
                    ::Smp::Int64 upper);

  /// Virtual destructor to release memory.
  ~AbstractContainer() noexcept override = default;

  /// Return the name of the container.
  /// @return  Name of object.
  ::Smp::String8 GetName() const final;

  /// Return the description of the container.
  /// Descriptions are optional and may be empty.
  /// @return  Description of container.
  ::Smp::String8 GetDescription() const final;

  /// Returns the parent object of the container.
  /// @return  Parent object of container or nullptr if container has no parent.
  ::Smp::IObject *GetParent() const final;

  /// Query for the collection of all components in the container.
  /// The returned collection may be empty if no components exist for the
  /// container.
  /// @return  Collection of contained components.
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

  /// Query for a component contained in the container by name.
  /// The returned component may be null if no child with the given name
  /// could be found.
  /// @param   name Component name.
  /// @return  Child component, or nullptr if no child component with the
  ///          given name exists.
  ::Smp::IComponent *GetComponent(::Smp::String8 name) const override = 0;

  /// Query for a component contained in the container by index.
  /// The returned component may be null if no child with the given name
  /// could be found.
  /// @param   index Component name.
  /// @return  Child component, or nullptr if no child component at the
  ///          given index exists.
  virtual ::Smp::IComponent *GetComponent(size_t index) const = 0;

  /// Add a contained component to the container.
  /// This method raises an exception of type ContainerFull if called for
  /// a full container, i.e. when the Count has reached the Upper limit.
  /// It raises an exception of type DuplicateName when trying to add a
  /// component with a name that is already contained in the container,
  /// as this would lead to duplicate names in the container. This method
  /// may raise an exception of type InvalidObjectType when it expects
  /// the given component to implement another interface as well.
  /// @param   component New contained component.
  /// @throws  Smp::ContainerFull
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectType
  void AddComponent(::Smp::IComponent *component) override;

  /// Delete a contained component from the container, and from memory.
  /// This method raises an exception of type NotContained if called for
  /// a component that has not been added to the container before.
  /// If the container contains the minimum number of components, the
  /// method throws a CannotDelete exception.
  /// @param   component Component to delete from container, and from
  ///          memory.
  /// @throws  Smp::CannotDelete
  /// @throws  Smp::NotContained
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

    /// Retrieve element by name
    /// @param name The name of the element to be retrieved.
    /// @return Instance with given name, or nullptr of no such instance exists.
    ::Smp::IComponent *at(::Smp::String8 name) const override;

    /// Retrieve element by position in the sequence (based on order of
    /// insertion).
    /// @param index The position of the elment to be retrieved.
    /// @return Instance at given index, or nullptr of no instance exists.
    ::Smp::IComponent *at(size_t index) const override;

    /// Get the number of objects in the sequence.
    /// @return Number of objects in the sequence.
    size_t size() const override;

    /// Check if the collection is empty.
    /// @return true if the collection is empty, false otherwise.
    bool empty() const;

    /// Get the begin iterator
    /// @return Begin iterator
    const_iterator begin() const override;

    /// Get the end iterator
    /// @return End iterator
    const_iterator end() const override;

  private:
    AbstractContainer &_parent;
  };
  ::Xsmp::cstring _name;
  ::Xsmp::cstring _description;
  ::Smp::IObject *_parent;
  Collection _collection;
  ::Smp::Int64 _lower;
  ::Smp::Int64 _upper;
};

} // namespace detail

/// @class Container
/// XSMP implementation of ::Smp::IContainer.
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

  /// Constructs a Container object
  ///  @param name The name of the container.
  ///  @param description The description of the container.
  ///  @param parent The parent Container object.
  ///  @param lower The lower bound of the container.
  ///  @param upper The upper bound of thecontainer.
  Container(::Smp::String8 name, ::Smp::String8 description,
            ::Xsmp::Composite *parent, ::Smp::Int64 lower, ::Smp::Int64 upper)
      : AbstractContainer(name, description, parent, lower, upper) {}

  /// Constructs a Container object
  ///  @param name The name of the container.
  ///  @param description The description of the container.
  ///  @param parent The parent Container object.
  ///  @param lower The lower bound of the container.
  ///  @param upper The upper bound of thecontainer.
  Container(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, ::Smp::Int64 lower, ::Smp::Int64 upper)
      : AbstractContainer(name, description, parent, lower, upper) {}

  /// Virtual destructor to release memory.
  ~Container() noexcept override {
    // Clear the vector in reverse order
    while (!_vector.empty()) {
      delete _vector.back();
      _vector.pop_back();
    }
  }

  /// Query for a component contained in the container by name.
  /// The returned component may be null if no child with the given name
  /// could be found.
  /// @param   name Component name.
  /// @return  Child component, or null if no child component with the
  ///          given name exists.
  ::Smp::IComponent *GetComponent(::Smp::String8 name) const override {
    if (name) {
      if (auto it = find(name); it != end()) {
        return dynamic_cast<::Smp::IComponent *>(*it);
      }
    }
    return nullptr;
  }

  /// Query for a component contained in the container by index.
  /// The returned component may be null if no child with the given name
  /// could be found.
  /// @param   index Component name.
  /// @return  Child component, or nullptr if no child component at the
  ///          given index exists.
  ::Smp::IComponent *GetComponent(size_t index) const override {
    if (index < size()) {
      return dynamic_cast<::Smp::IComponent *>(_vector[index]);
    }
    return nullptr;
  }

  /// Add a contained component to the container.
  /// This method raises an exception of type ContainerFull if called for
  /// a full container, i.e. when the Count has reached the Upper limit.
  /// It raises an exception of type DuplicateName when trying to add a
  /// component with a name that is already contained in the container,
  /// as this would lead to duplicate names in the container. This method
  /// may raise an exception of type InvalidObjectType when it expects
  /// the given component to implement another interface as well.
  /// @param   component New contained component.
  /// @throws  Smp::ContainerFull
  /// @throws  Smp::DuplicateName
  /// @throws  Smp::InvalidObjectType
  void AddComponent(::Smp::IComponent *component) override {
    AbstractContainer::AddComponent(component);
    // check that the component type can be casted to T
    if (auto *casted = dynamic_cast<T *>(component)) {
      _vector.emplace_back(casted);
    } else {
      ::Xsmp::Exception::throwInvalidObjectType<T>(this, component);
    }
  }

  /// Delete a contained component from the container, and from memory.
  /// This method raises an exception of type NotContained if called for
  /// a component that has not been added to the container before.
  /// If the container contains the minimum number of components, the
  /// method throws a CannotDelete exception.
  /// @param   component Component to delete from container, and from
  ///          memory.
  /// @throws  Smp::CannotDelete
  /// @throws  Smp::NotContained
  void DeleteComponent(::Smp::IComponent *component) override {
    if (auto it = find(dynamic_cast<T *>(component)); it != end()) {
      AbstractContainer::DeleteComponent(component);
      _vector.erase(it);
    } else {
      ::Xsmp::Exception::throwNotContained(this, component);
    }
  }
  /// Query for the number of components in the collection.
  /// @return  Current number of components in the collection.
  ::Smp::Int64 GetCount() const override {
    return static_cast<::Smp::Int64>(size());
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
  holder _vector;
};

} // namespace Xsmp

#endif // XSMP_CONTAINER_H_
