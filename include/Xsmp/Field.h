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

#ifndef XSMP_FIELD_H_
#define XSMP_FIELD_H_

#include <Smp/AnySimple.h>
#include <Smp/AnySimpleArray.h>
#include <Smp/IArrayField.h>
#include <Smp/IDataflowField.h>
#include <Smp/IFailure.h>
#include <Smp/IField.h>
#include <Smp/IForcibleField.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/IStorageReader.h>
#include <Smp/IStorageWriter.h>
#include <Smp/IStructureField.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IArrayType.h>
#include <Smp/Publication/IStructureType.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/AnySimpleConverter.h>
#include <Xsmp/Array.h>
#include <Xsmp/Collection.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/cstring.h>
#include <cstddef>
#include <iterator>
#include <type_traits>
#include <utility>

namespace Smp {
class IComponent;
} // namespace Smp

/// XSMP standard types and interfaces.
namespace Xsmp {

namespace detail {

template <typename, class U = void> struct FieldTypeHelper {};
} // namespace detail

/// Alias template to simplify the definition of a field based on the template
/// parameters T and Annotations.
/// The field can be specialized by appending one or more of:
/// - ::transient: The field is transient (not persisted). Example:
/// Field<T>::transient
/// - ::input: The field is an input. Example: Field<T>::input
/// - ::output: The field is an output. Example: Field<T>::output
/// - ::failure: The field is a failure. Example: Field<T>::failure
/// - ::forcible: The field is forcible. Example: Field<T>::forcible
/// The transient, input, output, forcible and failure specialization can be
/// combined (e.g: Field<T>::transient::input).
/// @tparam T The type of data stored in the field: can be a SimpleType,
/// ArrayType (simple or not), or StructureType.
/// @tparam Annotations Zero or more annotations that modify the behavior of the
/// field (see `Xsmp::Annotation` namespace for available annotations).
template <typename T, typename... Annotations>
using Field =
    typename ::Xsmp::detail::FieldTypeHelper<T>::template field<Annotations...>;

/// XSMP annotations.
namespace Annotation {

template <typename Annotation, typename... Annotations>
static constexpr bool any_of =
    std::disjunction_v<std::is_same<Annotation, Annotations>...>;

/// Annotation indicating that a Field is transient (not a state)
struct transient {};

/// Annotation indicating that a Field is an input
struct input {};

/// Annotation indicating that a Field is an output
struct output {};

/// Annotation indicating that a Field is forcible
struct forcible {};

/// Annotation indicating that a Field is a failure
struct failure {};

/// Annotation indicating that a Field is connectable
/// Indicate that a SimpleField or SimpleArrayField can be connected
struct connectable {};

} // namespace Annotation
class Component;

namespace detail {

template <typename T = ::Smp::Publication::IType>
const T *GetType(const ::Smp::IField *field,
                 const ::Smp::Publication::ITypeRegistry *typeRegistry,
                 ::Smp::Uuid typeUuid) {
  if constexpr (std::is_same_v<T, ::Smp::Publication::IType>) {
    if (const auto *type = typeRegistry->GetType(typeUuid)) {
      return type;
    }
  } else {
    if (const auto *type =
            dynamic_cast<const T *>(typeRegistry->GetType(typeUuid))) {
      return type;
    }
  }
  ::Xsmp::Exception::throwInvalidFieldType(field, typeUuid);
}

template <class T, class = void>
struct is_simple_array_field : std::false_type {};

template <typename ItemType, std::size_t Nm, typename... Annotations>
struct is_simple_array_field<
    ::Xsmp::Array<ItemType, Nm, Annotations...>,
    std::enable_if_t<::Xsmp::Annotation::any_of<::Xsmp::Annotation::simpleArray,
                                                Annotations...>
                         && ::Xsmp::Helper::is_simple_type_v<ItemType>>>
    : std::true_type {};

class FieldHelper;

class AbstractForcibleField : public virtual ::Smp::IForcibleField {
public:
  void Force(::Smp::AnySimple value) final;
  void Unforce() final;
  ::Smp::Bool IsForced() final;
  void Freeze() final;
  void Restore(::Smp::IStorageReader *reader) override;
  void Store(::Smp::IStorageWriter *writer) override;

private:
  bool _forced{};
};
template <typename T> class ForcibleField : public AbstractForcibleField {
public:
  using AbstractForcibleField::Force;
  void Force(const T &value) {
    Force(::Xsmp::AnySimpleConverter<T>::convert(this->GetPrimitiveTypeKind(),
                                                 value));
  }
};
template <typename T, typename... Annotations>
using ForcibleFieldType = std::conditional_t<
    ::Xsmp::Annotation::any_of<::Xsmp::Annotation::forcible, Annotations...>,
    ::Xsmp::detail::ForcibleField<T>, ::Smp::ISimpleField>;

class Failure : public virtual ::Smp::IFailure {
public:
  void Fail() final;
  void Unfail() final;
  ::Smp::Bool IsFailed() const final;
  void Restore(::Smp::IStorageReader *reader) override;
  void Store(::Smp::IStorageWriter *writer) override;

protected:
  void Register();

private:
  bool _failed{};
};
class IDataflowFieldExtension : public virtual ::Smp::IDataflowField {
public:
  /// Virtual destructor to release memory.
  ~IDataflowFieldExtension() noexcept override = default;

  /// Disconnect this field to a target field for direct data flow.
  /// @param   target Target field to connect to. The field type must be
  ///          compatible.
  virtual void Disconnect(const ::Smp::IField *target) = 0;

  virtual const ::Smp::FieldCollection *GetInputFields() const {
    return nullptr;
  };
};

class InputFieldCollection final : public ::Xsmp::Object,
                                   public ::Smp::FieldCollection {
public:
  using ::Xsmp::Object::Object;
  using const_iterator = typename ::Smp::FieldCollection::const_iterator;
  using iterator = typename ::Smp::FieldCollection::iterator;
  ::Smp::IField *at(::Smp::String8 name) const override;
  ::Smp::IField *at(size_t index) const override;
  size_t size() const override;
  virtual bool empty() const;
  const_iterator begin() const override;
  const_iterator end() const override;
  bool contains(const ::Smp::IField *input) const;
  void add(::Smp::IField *input);
  bool remove(const ::Smp::IField *input);

private:
  std::vector<::Smp::IField *> _fields;
};
class ArrayDataflowField : public virtual IDataflowFieldExtension,
                           public virtual ::Smp::IArrayField {
public:
  ArrayDataflowField();
  void Push() final;
  void Connect(::Smp::IField *target) final;
  void Disconnect(const ::Smp::IField *target) final;
  const ::Smp::FieldCollection *GetInputFields() const final;

private:
  InputFieldCollection _connectedFields;
  friend class ::Xsmp::detail::FieldHelper;
};

template <typename... Annotations>
using ArrayField = std::conditional_t<
    ::Xsmp::Annotation::any_of<::Xsmp::Annotation::output, Annotations...>,
    ::Xsmp::detail::ArrayDataflowField, ::Smp::IArrayField>;

class SimpleConnectableField : public virtual ::Smp::ISimpleField {
protected:
  SimpleConnectableField();
  void internal_push() const;
  virtual const ::Smp::FieldCollection *GetInputFields() const;

private:
  InputFieldCollection _connectedFields;
  friend class ::Xsmp::detail::FieldHelper;
};
class SimpleDataflowField : public virtual SimpleConnectableField,
                            public virtual IDataflowFieldExtension {
public:
  void Push() final;
  void Connect(::Smp::IField *target) final;
  void Disconnect(const ::Smp::IField *target) final;
  const ::Smp::FieldCollection *GetInputFields() const final;
};
template <typename... Annotations>
using SimpleField = std::conditional_t<
    ::Xsmp::Annotation::any_of<::Xsmp::Annotation::output, Annotations...>,
    SimpleDataflowField,
    std::conditional_t<::Xsmp::Annotation::any_of<
                           ::Xsmp::Annotation::connectable, Annotations...>,
                       SimpleConnectableField, ::Smp::IField>>;

class SimpleArrayConnectableField : public virtual ::Smp::ISimpleArrayField {
protected:
  SimpleArrayConnectableField();
  void internal_push(::Smp::UInt64 index) const;
  virtual const ::Smp::FieldCollection *GetInputFields() const;

private:
  InputFieldCollection _connectedFields;
  friend class ::Xsmp::detail::FieldHelper;
};

class SimpleArrayDataflowField : public virtual SimpleArrayConnectableField,
                                 public virtual IDataflowFieldExtension {
public:
  void Push() final;
  void Connect(::Smp::IField *target) final;
  void Disconnect(const ::Smp::IField *target) final;
  const ::Smp::FieldCollection *GetInputFields() const final;
};
template <typename... Annotations>
using SimpleArrayField = std::conditional_t<
    ::Xsmp::Annotation::any_of<::Xsmp::Annotation::output, Annotations...>,
    SimpleArrayDataflowField,
    std::conditional_t<::Xsmp::Annotation::any_of<
                           ::Xsmp::Annotation::connectable, Annotations...>,
                       SimpleArrayConnectableField, ::Smp::ISimpleArrayField>>;

class AbstractField : public virtual ::Smp::IField {
public:
  AbstractField(const AbstractField &) = delete;
  AbstractField &operator=(const AbstractField &) = delete;
  ~AbstractField() noexcept override = default;
  ::Smp::String8 GetName() const final;
  ::Smp::String8 GetDescription() const final;
  ::Smp::IObject *GetParent() const final;
  ::Smp::ViewKind GetView() const final;

protected:
  AbstractField(::Smp::String8 name, ::Smp::String8 description,
                ::Smp::IObject *parent, ::Smp::ViewKind view);
  friend class ::Xsmp::Component;

private:
  ::Xsmp::cstring _name;
  ::Xsmp::cstring _description;
  ::Smp::IObject *_parent;
  ::Smp::ViewKind _view;
};

class AbstractStructureField : public virtual ::Smp::IStructureField {
public:
  AbstractStructureField();
  void Restore(::Smp::IStorageReader *reader) override;
  void Store(::Smp::IStorageWriter *writer) override;
  const ::Smp::FieldCollection *GetFields() const final;
  ::Smp::IField *GetField(::Smp::String8 name) const final;

protected:
  ::Xsmp::Collection<::Smp::IField> *GetFields();

  friend class ::Xsmp::detail::AbstractField;
  inline void AddField(::Smp::IField &field) { _fields.Add(&field); }

private:
  ::Xsmp::Collection<::Smp::IField> _fields;
};

class StructureDataflowField : public ::Xsmp::detail::AbstractStructureField,
                               public virtual IDataflowFieldExtension {
public:
  StructureDataflowField();
  void Push() final;
  void Connect(::Smp::IField *target) final;
  void Disconnect(const ::Smp::IField *target) final;
  const ::Smp::FieldCollection *GetInputFields() const final;

private:
  InputFieldCollection _connectedFields;
  friend class ::Xsmp::detail::FieldHelper;
};
template <typename... Annotations>
using StructureField = std::conditional_t<
    ::Xsmp::Annotation::any_of<::Xsmp::Annotation::output, Annotations...>,
    ::Xsmp::detail::StructureDataflowField,
    ::Xsmp::detail::AbstractStructureField>;

template <typename T, typename... Annotations>
class Field : public AbstractField,
              public virtual std::conditional_t<
                  ::Xsmp::Annotation::any_of<::Xsmp::Annotation::failure,
                                             Annotations...>,
                  Failure, ::Smp::IPersist> {

public:
  using transient =
      ::Xsmp::Field<T, Annotations..., ::Xsmp::Annotation::transient>;
  using input = ::Xsmp::Field<T, Annotations..., ::Xsmp::Annotation::input>;
  using output = ::Xsmp::Field<T, Annotations..., ::Xsmp::Annotation::output,
                               ::Xsmp::Annotation::connectable>;
  using connectable =
      ::Xsmp::Field<T, Annotations..., ::Xsmp::Annotation::connectable>;
  using forcible =
      ::Xsmp::Field<T, Annotations..., ::Xsmp::Annotation::forcible>;
  using failure = ::Xsmp::Field<T, Annotations..., ::Xsmp::Annotation::failure>;

  Field(::Smp::String8 name, ::Smp::String8 description, ::Smp::IObject *parent,
        ::Smp::ViewKind view)
      : AbstractField(name, description, parent, view) {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::failure,
                                             Annotations...>) {
      this->Register();
    }
  }

  ::Smp::Bool IsState() const final {
    return !::Xsmp::Annotation::any_of<::Xsmp::Annotation::transient,
                                       Annotations...>;
  }
  ::Smp::Bool IsInput() const final {
    return ::Xsmp::Annotation::any_of<::Xsmp::Annotation::input,
                                      Annotations...>;
  }
  ::Smp::Bool IsOutput() const final {
    return ::Xsmp::Annotation::any_of<::Xsmp::Annotation::output,
                                      Annotations...>;
  }

  void Restore(::Smp::IStorageReader *reader) override {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::failure,
                                             Annotations...>) {
      Failure::Restore(reader);
    }
  }
  void Store(::Smp::IStorageWriter *writer) override {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::failure,
                                             Annotations...>) {
      Failure::Store(writer);
    }
  }
};
} // namespace detail

template <typename T, typename... Annotations>
class SimpleField final
    : public ::Xsmp::detail::Field<T, Annotations...>,
      public virtual ::Xsmp::detail::ForcibleFieldType<T, Annotations...>,
      public virtual ::Xsmp::detail::SimpleField<Annotations...> {

  using base_class = ::Xsmp::detail::Field<T, Annotations...>;

public:
  SimpleField(const SimpleField &) = delete;
  SimpleField &operator=(const SimpleField &) = delete;
  SimpleField(::Smp::Publication::ITypeRegistry *typeRegistry,
              ::Smp::Uuid typeUuid, ::Smp::String8 name,
              ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
              ::Smp::ViewKind view = ::Smp::ViewKind::VK_None,
              const T &value = {})
      : ::Xsmp::detail::Field<T, Annotations...>(name, description, parent,
                                                 view),
        _type{::Xsmp::detail::GetType<>(this, typeRegistry, typeUuid)},
        _value{value} {}

  ::Smp::PrimitiveTypeKind GetPrimitiveTypeKind() const override {
    return _type->GetPrimitiveTypeKind();
  }

  ::Smp::AnySimple GetValue() const override {
    return ::Xsmp::AnySimpleConverter<T>::convert(GetPrimitiveTypeKind(),
                                                  _value);
  }

  void SetValue(::Smp::AnySimple value) override {
    SetValue(::Xsmp::AnySimpleConverter<T>::convert(value));
  }

  void Restore(::Smp::IStorageReader *reader) override {
    if constexpr (!::Xsmp::Annotation::any_of<::Xsmp::Annotation::transient,
                                              Annotations...>) {
      reader->Restore(&_value, sizeof(_value));
      if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::forcible,
                                               Annotations...>) {
        ::Xsmp::detail::ForcibleField<T>::Restore(reader);
      }
      base_class::Restore(reader);
    }
  }
  void Store(::Smp::IStorageWriter *writer) override {
    if constexpr (!::Xsmp::Annotation::any_of<::Xsmp::Annotation::transient,
                                              Annotations...>) {
      writer->Store(&_value, sizeof(_value));
      if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::forcible,
                                               Annotations...>) {
        ::Xsmp::detail::ForcibleField<T>::Store(writer);
      }
      base_class::Store(writer);
    }
  }
  const ::Smp::Publication::IType *GetType() const override { return _type; }

  using reference =
      std::conditional_t<::Xsmp::Annotation::any_of<
                             ::Xsmp::Annotation::connectable, Annotations...> ||
                             ::Xsmp::Annotation::any_of<
                                 ::Xsmp::Annotation::forcible, Annotations...>,
                         const T &, T &>;
  using const_reference = const T &;

  using pointer =
      std::conditional_t<::Xsmp::Annotation::any_of<
                             ::Xsmp::Annotation::connectable, Annotations...> ||
                             ::Xsmp::Annotation::any_of<
                                 ::Xsmp::Annotation::forcible, Annotations...>,
                         const T *, T *>;
  using const_pointer = const T *;
  operator const_reference() const noexcept { return _value; }
  operator reference() noexcept { return _value; }

  const_reference operator*() const noexcept { return _value; }
  reference operator*() noexcept { return _value; }

  const_pointer operator->() const noexcept { return &_value; }
  pointer operator->() noexcept { return &_value; }

  SimpleField &operator=(const T &value) noexcept { return SetValue(value); }

  SimpleField &operator++() noexcept { return SetValue(_value + 1); }
  SimpleField &operator--() noexcept { return SetValue(_value - 1); }
  template <typename U> SimpleField &operator<<=(const U &_i) noexcept {
    return SetValue(_value << _i);
  }
  template <typename U> SimpleField &operator>>=(const U &_i) noexcept {
    return SetValue(_value >> _i);
  }
  template <typename U> SimpleField &operator+=(const U &_i) noexcept {
    return SetValue(_value + _i);
  }
  template <typename U> SimpleField &operator-=(const U &_i) noexcept {
    return SetValue(_value - _i);
  }
  template <typename U> SimpleField &operator*=(const U &_i) noexcept {
    return SetValue(_value * _i);
  }
  template <typename U> SimpleField &operator/=(const U &_i) noexcept {
    return SetValue(_value / _i);
  }
  template <typename U> SimpleField &operator%=(const U &_i) noexcept {
    return SetValue(_value % _i);
  }
  template <typename U> SimpleField &operator&=(const U &_i) noexcept {
    return SetValue(_value & _i);
  }
  template <typename U> SimpleField &operator|=(const U &_i) noexcept {
    return SetValue(_value | _i);
  }
  template <typename U> SimpleField &operator^=(const U &_i) noexcept {
    return SetValue(_value ^ _i);
  }

private:
  inline SimpleField &SetValue(const T &value) {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::forcible,
                                             Annotations...>) {
      if (this->IsForced()) {
        return *this; // do not change the value
      }
    }
    _value = value;
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::connectable,
                                             Annotations...>) {
      this->internal_push();
    }
    return *this;
  }

  const ::Smp::Publication::IType *_type;
  T _value;
};

template <typename T, typename... Annotations>
class ArrayField final
    : public ::Xsmp::detail::Field<T, Annotations...>,
      public virtual ::Xsmp::detail::ArrayField<Annotations...> {

  static constexpr size_t _size = ::Xsmp::detail::FieldTypeHelper<T>::size;

  template <typename... Options> struct apply_options {
    template <typename U> struct on {
      using type = U;
    };
  };
  template <typename Option, typename... Options>
  struct apply_options<Option, Options...> {
    template <typename U, typename Annotation> struct annotation {
      using type = U;
    };

    template <typename U> struct annotation<U, ::Xsmp::Annotation::output> {
      using type = typename U::output;
    };
    template <typename U>
    struct annotation<U, ::Xsmp::Annotation::connectable> {
      using type = typename U::connectable;
    };
    template <typename U> struct annotation<U, ::Xsmp::Annotation::input> {
      using type = typename U::input;
    };

    template <typename U> struct annotation<U, ::Xsmp::Annotation::forcible> {
      using type = typename U::forcible;
    };
    template <typename U> struct on {
      using type = typename apply_options<Options...>::template on<
          typename annotation<U, Option>::type>::type;
    };
  };

public:
  ArrayField(const ArrayField &) = delete;
  ArrayField &operator=(const ArrayField &) = delete;
  ArrayField(::Smp::Publication::ITypeRegistry *typeRegistry,
             ::Smp::Uuid typeUuid, ::Smp::String8 name,
             ::Smp::String8 description = "", ::Smp::IObject *parent = nullptr,
             ::Smp::ViewKind view = ::Smp::ViewKind::VK_None,
             const T &value = {})
      : ::Xsmp::detail::Field<T, Annotations...>(name, description, parent,
                                                 view),
        _type{::Xsmp::detail::GetType<::Smp::Publication::IArrayType>(
            this, typeRegistry, typeUuid)},
        internalArray{create_array_impl(typeRegistry, value,
                                        std::make_index_sequence<_size>{})} {}

  ::Smp::UInt64 GetSize() const override { return _size; }

  ::Smp::IField *GetItem(::Smp::UInt64 index) const override {
    if (index >= _size) {
      ::Xsmp::Exception::throwInvalidArrayIndex(this, index);
    }
    return const_cast<value_type *>(&internalArray[index]);
  }
  const ::Smp::Publication::IArrayType *GetType() const override {
    return _type;
  }

  void Restore(::Smp::IStorageReader *reader) override {
    if constexpr (!::Xsmp::Annotation::any_of<::Xsmp::Annotation::transient,
                                              Annotations...>) {
      std::for_each(internalArray.begin(), internalArray.end(),
                    [reader](auto &field) { field.Restore(reader); });
      ::Xsmp::detail::Field<T, Annotations...>::Restore(reader);
    }
  }
  void Store(::Smp::IStorageWriter *writer) override {
    if constexpr (!::Xsmp::Annotation::any_of<::Xsmp::Annotation::transient,
                                              Annotations...>) {
      std::for_each(internalArray.begin(), internalArray.end(),
                    [writer](auto &field) { field.Store(writer); });
      ::Xsmp::detail::Field<T, Annotations...>::Store(writer);
    }
  }
  operator T() const noexcept {
    T value;
    std::copy(this->cbegin(), this->cend(), value.begin());
    return value;
  }
  ArrayField &operator=(const T &other) {
    std::copy(other.cbegin(), other.cend(), this->begin());
    return *this;
  }
  using value_type = typename apply_options<Annotations...>::template on<
      ::Xsmp::Field<typename T::value_type>>::type;
  using pointer = value_type *;
  using const_pointer = const value_type *;
  using reference = value_type &;
  using const_reference = const value_type &;
  using iterator = value_type *;
  using const_iterator = const value_type *;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  void fill(const typename T::value_type &_u) { internalArray.fill(_u); }

  // Iterators.
  [[nodiscard]] iterator begin() noexcept { return internalArray.begin(); }

  [[nodiscard]] const_iterator begin() const noexcept {
    return internalArray.begin();
  }

  [[nodiscard]] iterator end() noexcept { return internalArray.end(); }

  [[nodiscard]] const_iterator end() const noexcept {
    return internalArray.end();
  }

  [[nodiscard]] reverse_iterator rbegin() noexcept {
    return internalArray.rbegin();
  }

  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return internalArray.rbegin();
  }

  [[nodiscard]] reverse_iterator rend() noexcept {
    return internalArray.rend();
  }

  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return internalArray.rend();
  }

  [[nodiscard]] const_iterator cbegin() const noexcept {
    return internalArray.cbegin();
  }

  [[nodiscard]] const_iterator cend() const noexcept {
    return internalArray.cend();
  }

  [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
    return internalArray.crbegin();
  }

  [[nodiscard]] const_reverse_iterator crend() const noexcept {
    return internalArray.crend();
  }

  // Capacity.
  [[nodiscard]] size_type size() const noexcept { return _size; }

  [[nodiscard]] size_type max_size() const noexcept { return _size; }

  [[nodiscard]] bool empty() const noexcept { return _size == 0; }

  // Element access.
  [[nodiscard]] reference operator[](size_type index) noexcept {
    return internalArray[index];
  }

  [[nodiscard]] const_reference operator[](size_type index) const noexcept {
    return internalArray[index];
  }

  reference at(size_type index) { return internalArray.at(index); }

  const_reference at(size_type index) const { return internalArray.at(index); }

  [[nodiscard]] reference front() noexcept { return internalArray.front(); }

  [[nodiscard]] const_reference front() const noexcept {
    return internalArray.front();
  }

  [[nodiscard]] reference back() noexcept { return internalArray.back(); }

  [[nodiscard]] const_reference back() const noexcept {
    return internalArray.back();
  }

  [[nodiscard]] pointer data() noexcept { return internalArray.data(); }

  [[nodiscard]] const_pointer data() const noexcept {
    return internalArray.data();
  }

private:
  template <size_t... I>
  ::Xsmp::Array<value_type, _size>
  create_array_impl(::Smp::Publication::ITypeRegistry *typeRegistry,
                    const T &value, std::index_sequence<I...>) {
    return ::Xsmp::Array<value_type, _size>{
        {value_type(typeRegistry, GetType()->GetItemType()->GetUuid(),
                    ("[" + std::to_string(I) + "]").c_str(), "", this,
                    this->GetView(), value[I])...}};
  }

  const ::Smp::Publication::IArrayType *_type;

  ::Xsmp::Array<value_type, _size> internalArray;
};

template <typename T, typename... Annotations>
class SimpleArrayField final
    : public ::Xsmp::detail::Field<T, Annotations...>,
      public virtual ::Xsmp::detail::SimpleArrayField<Annotations...> {
  static_assert(::Xsmp::Helper::is_simple_type_v<typename T::value_type>,
                "Only Smp Simple types are supported.");
  static_assert(
      !::Xsmp::Annotation::any_of<::Xsmp::Annotation::forcible, Annotations...>,
      "A SimpleArrayField cannot be forcible.");
  static constexpr size_t _size = ::Xsmp::detail::FieldTypeHelper<T>::size;

public:
  SimpleArrayField(const SimpleArrayField &) = delete;
  SimpleArrayField &operator=(const SimpleArrayField &) = delete;
  SimpleArrayField(::Smp::Publication::ITypeRegistry *typeRegistry,
                   ::Smp::Uuid typeUuid, ::Smp::String8 name,
                   ::Smp::String8 description = "",
                   ::Smp::IObject *parent = nullptr,
                   ::Smp::ViewKind view = ::Smp::ViewKind::VK_None,
                   const T &value = {})
      : ::Xsmp::detail::Field<T, Annotations...>(name, description, parent,
                                                 view),
        _type{::Xsmp::detail::GetType<::Smp::Publication::IArrayType>(
            this, typeRegistry, typeUuid)},
        _value{value} {}

  ::Smp::UInt64 GetSize() const override { return _size; }
  ::Smp::AnySimple GetValue(::Smp::UInt64 index) const override {
    if (index >= _size) {
      ::Xsmp::Exception::throwInvalidArrayIndex(this, index);
    }
    return ::Xsmp::AnySimpleConverter<value_type>::convert(
        _type->GetItemType()->GetPrimitiveTypeKind(), _value[index]);
  }

  void SetValue(::Smp::UInt64 index, ::Smp::AnySimple value) override {
    if (index >= _size) {
      ::Xsmp::Exception::throwInvalidArrayIndex(this, index);
    }
    if (_type->GetItemType()->GetPrimitiveTypeKind() != value.type) {
      ::Xsmp::Exception::throwInvalidFieldValue(this, value);
    }
    _value[index] = ::Xsmp::AnySimpleConverter<value_type>::convert(value);
  }

  void GetValues(::Smp::UInt64 length,
                 ::Smp::AnySimpleArray values) const override {
    if (length != _size) {
      ::Xsmp::Exception::throwInvalidArraySize(this, length);
    }
    auto _itemKind = _type->GetItemType()->GetPrimitiveTypeKind();
    for (size_t i = 0; i < _size; ++i) {
      values[i] =
          ::Xsmp::AnySimpleConverter<value_type>::convert(_itemKind, _value[i]);
    }
  }

  void SetValues(::Smp::UInt64 length, ::Smp::AnySimpleArray values) override {
    if (length != _size) {
      ::Xsmp::Exception::throwInvalidArraySize(this, length);
    }
    auto _itemKind = _type->GetItemType()->GetPrimitiveTypeKind();
    for (size_t i = 0; i < _size; ++i) {
      if (values[i].type != _itemKind) {
        ::Xsmp::Exception::throwInvalidArrayValue(this, i, values[i]);
      }
      _value[i] = ::Xsmp::AnySimpleConverter<value_type>::convert(values[i]);
    }
  }

  void Restore(::Smp::IStorageReader *reader) override {
    if constexpr (!::Xsmp::Annotation::any_of<::Xsmp::Annotation::transient,
                                              Annotations...>) {
      reader->Restore(_value.data(), sizeof(_value));
      ::Xsmp::detail::Field<T, Annotations...>::Restore(reader);
    }
  }
  void Store(::Smp::IStorageWriter *writer) override {
    if constexpr (!::Xsmp::Annotation::any_of<::Xsmp::Annotation::transient,
                                              Annotations...>) {
      writer->Store(_value.data(), sizeof(_value));
      ::Xsmp::detail::Field<T, Annotations...>::Store(writer);
    }
  }
  const ::Smp::Publication::IArrayType *GetType() const override {
    return _type;
  }

  using value_type = typename T::value_type;
  using pointer = value_type *;
  using const_pointer = const value_type *;

  struct protected_reference {
    protected_reference(SimpleArrayField *parent, value_type &value,
                        size_t index)
        : _parent{parent}, _value{value}, _index{index} {}

    operator const T &() const noexcept { return _value; }

    const T &operator*() const noexcept { return _value; }

    const T &operator->() const noexcept { return &_value; }

    protected_reference &operator=(const value_type &value) noexcept {
      return SetValue(value);
    }

    protected_reference &operator++() noexcept { return SetValue(_value + 1); }
    protected_reference &operator--() noexcept { return SetValue(_value - 1); }
    template <typename U>
    protected_reference &operator<<=(const U &_i) noexcept {
      return SetValue(_value << _i);
    }
    template <typename U>
    protected_reference &operator>>=(const U &_i) noexcept {
      return SetValue(_value >> _i);
    }
    template <typename U>
    protected_reference &operator+=(const U &_i) noexcept {
      return SetValue(_value + _i);
    }
    template <typename U>
    protected_reference &operator-=(const U &_i) noexcept {
      return SetValue(_value - _i);
    }
    template <typename U>
    protected_reference &operator*=(const U &_i) noexcept {
      return SetValue(_value * _i);
    }
    template <typename U>
    protected_reference &operator/=(const U &_i) noexcept {
      return SetValue(_value / _i);
    }
    template <typename U>
    protected_reference &operator%=(const U &_i) noexcept {
      return SetValue(_value % _i);
    }
    template <typename U>
    protected_reference &operator&=(const U &_i) noexcept {
      return SetValue(_value & _i);
    }
    template <typename U>
    protected_reference &operator|=(const U &_i) noexcept {
      return SetValue(_value | _i);
    }
    template <typename U>
    protected_reference &operator^=(const U &_i) noexcept {
      return SetValue(_value ^ _i);
    }

  private:
    inline protected_reference &SetValue(const value_type &value) noexcept {
      _value = value;
      _parent->internal_push(_index);
      return *this;
    }
    SimpleArrayField *_parent;
    value_type &_value;
    size_t _index;
  };

  using reference =
      std::conditional_t<::Xsmp::Annotation::any_of<
                             ::Xsmp::Annotation::connectable, Annotations...>,
                         protected_reference, value_type &>;
  using const_reference = const value_type &;
  using size_type = size_t;
  using difference_type = std::ptrdiff_t;
  struct protected_iterator {
    using iterator_category = std::forward_iterator_tag;
    using difference_type = std::ptrdiff_t;
    using value_type = SimpleArrayField::value_type;
    using pointer = protected_iterator *;
    using reference = SimpleArrayField::reference;

    protected_iterator(SimpleArrayField *parent, value_type *value,
                       size_t index)
        : _parent{parent}, _value{value}, _index{index} {}
    friend inline bool operator==(const protected_iterator &lhs,
                                  const protected_iterator &rhs) {
      return lhs._value == rhs._value;
    }
    friend inline bool operator!=(const protected_iterator &lhs,
                                  const protected_iterator &rhs) {
      return lhs._value != rhs._value;
    }
    reference operator*() const noexcept {
      return reference{_parent, *_value, _index};
    }
    pointer operator->() const noexcept {
      return pointer{_parent, *_value, _index};
    }
    protected_iterator &operator++() noexcept {
      ++_value;
      ++_index;
      return *this;
    }
    protected_iterator operator++(int) noexcept {
      return protected_iterator(_parent, _value++, _index++);
    }
    // Bidirectional iterator requirements
    protected_iterator &operator--() noexcept {
      --_value;
      --_index;
      return *this;
    }
    protected_iterator operator--(int) noexcept {
      return protected_iterator(_parent, _value--, _index--);
    }
    // Random access iterator requirements
    reference operator[](difference_type index) const noexcept {
      return {_parent, _value[index], _index + index};
    }
    protected_iterator &operator+=(difference_type index) noexcept {
      _value += index;
      _index += index;
      return *this;
    }
    friend protected_iterator operator+(const protected_iterator &lhs,
                                        difference_type index) noexcept {
      return protected_iterator(lhs._parent, lhs._value + index,
                                lhs._index + index);
    }
    protected_iterator &operator-=(difference_type index) noexcept {
      _value -= index;
      _index -= index;
      return *this;
    }
    friend protected_iterator operator-(const protected_iterator &lhs,
                                        difference_type index) noexcept {
      return protected_iterator(lhs._parent, lhs._value - index,
                                lhs._index - index);
    }
    const value_type *base() const noexcept { return _value; }

  private:
    SimpleArrayField *_parent;
    value_type *_value;
    size_t _index;
  };

  using iterator =
      std::conditional_t<::Xsmp::Annotation::any_of<
                             ::Xsmp::Annotation::connectable, Annotations...>,
                         protected_iterator, value_type *>;
  using const_iterator = const value_type *;
  using reverse_iterator = std::reverse_iterator<iterator>;
  using const_reverse_iterator = std::reverse_iterator<const_iterator>;

  operator const T &() const noexcept { return _value; }
  operator T &() noexcept { return _value; }

  void fill(const value_type &_u) { _value.fill(_u); }
  void swap(T &_other) noexcept { _value.swap(_other); }

  // Iterators.
  [[nodiscard]] iterator begin() noexcept {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::connectable,
                                             Annotations...>) {
      return iterator{this, _value.begin(), 0};
    } else {
      return _value.begin();
    }
  }
  [[nodiscard]] const_iterator begin() const noexcept { return _value.begin(); }

  [[nodiscard]] iterator end() noexcept {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::connectable,
                                             Annotations...>) {
      return iterator{this, _value.end(), _size};
    } else {
      return _value.end();
    }
  }

  [[nodiscard]] const_iterator end() const noexcept { return _value.end(); }

  [[nodiscard]] reverse_iterator rbegin() noexcept {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::connectable,
                                             Annotations...>) {
      return std::make_reverse_iterator(iterator{this, _value.end(), _size});
    } else {
      return _value.rbegin();
    }
  }

  [[nodiscard]] const_reverse_iterator rbegin() const noexcept {
    return _value.rbegin();
  }

  [[nodiscard]] reverse_iterator rend() noexcept {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::connectable,
                                             Annotations...>) {
      return std::make_reverse_iterator(iterator{this, _value.begin(), 0});
    } else {
      return _value.rend();
    }
  }

  [[nodiscard]] const_reverse_iterator rend() const noexcept {
    return _value.rend();
  }

  [[nodiscard]] const_iterator cbegin() const noexcept {
    return _value.cbegin();
  }

  [[nodiscard]] const_iterator cend() const noexcept { return _value.cend(); }

  [[nodiscard]] const_reverse_iterator crbegin() const noexcept {
    return _value.crbegin();
  }

  [[nodiscard]] const_reverse_iterator crend() const noexcept {
    return _value.crend();
  }

  // Capacity.
  [[nodiscard]] size_type size() const noexcept { return _size; }

  [[nodiscard]] size_type max_size() const noexcept { return _size; }

  [[nodiscard]] bool empty() const noexcept { return _size == 0; }

  // Element access.
  [[nodiscard]] reference operator[](size_type index) noexcept {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::connectable,
                                             Annotations...>) {
      return reference{this, _value[index], index};
    } else {
      return _value[index];
    }
  }

  [[nodiscard]] const_reference operator[](size_type index) const noexcept {
    return _value[index];
  }

  reference at(size_type index) {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::connectable,
                                             Annotations...>) {
      return reference{this, _value.at(index), index};
    } else {
      return _value.at(index);
    }
  }

  const_reference at(size_type index) const { return _value.at(index); }

  [[nodiscard]] reference front() noexcept {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::connectable,
                                             Annotations...>) {
      return reference{this, _value.front(), 0};
    } else {
      return _value.front();
    }
  }

  [[nodiscard]] const_reference front() const noexcept {
    return _value.front();
  }

  [[nodiscard]] reference back() noexcept {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::connectable,
                                             Annotations...>) {
      return reference{this, _value.back(), _size - 1};
    } else {
      return _value.back();
    }
  }

  [[nodiscard]] const_reference back() const noexcept { return _value.back(); }

  [[nodiscard]] pointer data() noexcept {
    if constexpr (::Xsmp::Annotation::any_of<::Xsmp::Annotation::connectable,
                                             Annotations...>) {
      return iterator{this, _value.data(), 0};
    } else {
      return _value.data();
    }
  }

  [[nodiscard]] const_pointer data() const noexcept { return _value.data(); }

private:
  const ::Smp::Publication::IArrayType *_type;
  T _value;
};
template <typename T, typename... Annotations>
class StructureField
    : public ::Xsmp::detail::Field<T, Annotations...>,
      public virtual ::Xsmp::detail::StructureField<Annotations...> {
public:
  // Constructor
  StructureField(::Smp::Publication::ITypeRegistry *typeRegistry,
                 ::Smp::Uuid typeUuid, ::Smp::String8 name,
                 ::Smp::String8 description, ::Smp::IObject *parent,
                 ::Smp::ViewKind view)
      : ::Xsmp::detail::Field<T, Annotations...>(name, description, parent,
                                                 view),

        _type{::Xsmp::detail::GetType<::Smp::Publication::IStructureType>(
            this, typeRegistry, typeUuid)} {}
  StructureField(const StructureField &) = delete;
  StructureField &operator=(const StructureField &) = delete;

  const ::Smp::Publication::IStructureType *GetType() const final {
    return _type;
  }

  void Restore(::Smp::IStorageReader *reader) final {
    if constexpr (!::Xsmp::Annotation::any_of<::Xsmp::Annotation::transient,
                                              Annotations...>) {
      ::Xsmp::detail::AbstractStructureField::Restore(reader);
      ::Xsmp::detail::Field<T, Annotations...>::Restore(reader);
    }
  }
  void Store(::Smp::IStorageWriter *writer) final {
    if constexpr (!::Xsmp::Annotation::any_of<::Xsmp::Annotation::transient,
                                              Annotations...>) {
      ::Xsmp::detail::AbstractStructureField::Store(writer);
      ::Xsmp::detail::Field<T, Annotations...>::Store(writer);
    }
  }

protected:
  template <typename... Options> struct apply_options {
    template <typename U> struct on {
      using type = U;
    };
  };
  template <typename Option, typename... Options>
  struct apply_options<Option, Options...> {
    template <typename U, typename Annotation> struct annotation {
      using type = U;
    };

    template <typename U> struct annotation<U, ::Xsmp::Annotation::output> {
      using type = typename U::connectable;
    };

    template <typename U> struct annotation<U, ::Xsmp::Annotation::forcible> {
      using type = typename U::forcible;
    };
    template <typename U> struct on {
      using type = typename apply_options<Options...>::template on<
          typename annotation<U, Option>::type>::type;
    };
  };

  template <typename U>
  using Field = typename apply_options<Annotations...>::template on<
      ::Xsmp::Field<U>>::type;

private:
  const ::Smp::Publication::IStructureType *_type;
};
namespace detail {

template <class, class = void> struct has_field : std::false_type {};

// specialization recognizes types that do have a nested ::type member:
template <class T>
struct has_field<T, std::void_t<typename T::template _Field<void>>>
    : std::true_type {};

template <typename T>
struct FieldTypeHelper<T, std::enable_if_t<has_field<T>::value>> {

  template <typename... Annotations>
  using field =
      typename T::template _Field<::Xsmp::StructureField<T, Annotations...>>;
};

template <typename T>
struct FieldTypeHelper<T,
                       std::enable_if_t<::Xsmp::Helper::is_simple_type_v<T>>> {
  template <typename... Annotations>
  using field = ::Xsmp::SimpleField<T, Annotations...>;
};

template <typename T, std::size_t Nm, typename... TypeAnnotations>
struct FieldTypeHelper<::Xsmp::Array<T, Nm, TypeAnnotations...>,

                       std::enable_if_t<!detail::is_simple_array_field<
                           ::Xsmp::Array<T, Nm, TypeAnnotations...>>::value>> {
  template <typename... Annotations>
  using field = ::Xsmp::ArrayField<::Xsmp::Array<T, Nm, TypeAnnotations...>,
                                   Annotations...>;
  static constexpr size_t size = Nm;
};

template <typename T, std::size_t Nm, typename... TypeAnnotations>
struct FieldTypeHelper<::Xsmp::Array<T, Nm, TypeAnnotations...>,

                       std::enable_if_t<detail::is_simple_array_field<
                           ::Xsmp::Array<T, Nm, TypeAnnotations...>>::value>> {
  template <typename... Annotations>
  using field =
      ::Xsmp::SimpleArrayField<::Xsmp::Array<T, Nm, TypeAnnotations...>,
                               Annotations...>;
  static constexpr size_t size = Nm;
};

} // namespace detail
} // namespace Xsmp

#endif // XSMP_FIELD_H_
