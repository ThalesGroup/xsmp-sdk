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

#ifndef XSMP_ARRAY_H_
#define XSMP_ARRAY_H_

#include <cstddef>
#include <cstdlib>
#include <iterator>
#include <stdexcept>
#include <string>
#include <type_traits>

/// XSMP standard types and interfaces.
namespace Xsmp {

/// XSMP annotations.
namespace Annotation {
/// Annotation indicating that a field of an array type implements
/// ISimpleArrayField instead of IArrayField
struct simpleArray {};
} // namespace Annotation

template <typename Tp, std::size_t Nm> struct _array_traits {
  using type = Tp[Nm];
  using is_nothrow_swappable = std::is_nothrow_swappable<Tp>;
};

template <typename Tp> struct _array_traits<Tp, 0> {
  // Empty type used instead of Tp[0] for Xsmp::Array<Tp, 0>.
  struct type {
    // Indexing is undefined.
    Tp &operator[](size_t) const noexcept { std::abort(); }

    // Conversion to a pointer produces a null pointer.
    constexpr explicit operator Tp *() const noexcept { return nullptr; }
  };
  using is_nothrow_swappable = std::true_type;
};

/// A SMP Array.
/// Contains an internalArray field as defined in ECSS SMP
/// Provides similar interfaces as a std::array
///
/// @tparam  Tp
///     Type of element.
/// @tparam  Nm
///     Number of elements.
/// @tparam  options
///     a list of options (only ::Xsmp::Annotation::simpleArray is supported).
template <typename Tp, std::size_t Nm, typename... options> struct Array {
  using simple = Array<Tp, Nm, ::Xsmp::Annotation::simpleArray, options...>;
  using value_type = Tp;
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

  typename _array_traits<Tp, Nm>::type internalArray;

  // No explicit construct/copy/destroy for aggregate type.

  /// Fills the array with a given value.
  /// @param value The value to fill the array with.
  constexpr void fill(const value_type &value) {
    // Use std::fill_n to set each element in the array to value
    std::fill_n(begin(), size(), value);
  }

  /// Swaps the contents of two arrays.
  /// @param other A reference to the other array to swap with.
  constexpr void swap(Array &other) noexcept(
      _array_traits<Tp, Nm>::is_nothrow_swappable::value) {
    // Use std::swap_ranges to swap elements of this array with those of another
    // array.
    std::swap_ranges(begin(), end(), other.begin());
  }

  /// Returns an iterator pointing to the first element in the array.
  /// @return An iterator pointing to the first element in the array.
  [[nodiscard]] constexpr iterator begin() noexcept { return iterator(data()); }

  /// Returns an iterator pointing to the first element in the array.
  /// @return An iterator pointing to the first element in the array.
  [[nodiscard]] constexpr const_iterator begin() const noexcept {
    return const_iterator(data());
  }

  /// Returns an iterator pointing to the past-the-end element in the array.
  /// @return An iterator pointing to the past-the-end element in the array.
  [[nodiscard]] constexpr iterator end() noexcept {
    return iterator(data() + Nm);
  }

  /// Returns an iterator pointing to the past-the-end element in the array.
  /// @return An iterator pointing to the past-the-end element in the array.
  [[nodiscard]] constexpr const_iterator end() const noexcept {
    return const_iterator(data() + Nm);
  }

  /// Returns a reverse iterator pointing to the last element in the array.
  /// @return A reverse iterator pointing to the last element in the array.
  [[nodiscard]] constexpr reverse_iterator rbegin() noexcept {
    return reverse_iterator(end());
  }

  /// Returns a reverse iterator pointing to the last element in the array
  /// @return A reverse iterator pointing to the last element in the array.
  [[nodiscard]] constexpr const_reverse_iterator rbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  /// Returns a reverse iterator pointing to one past the first element in the
  /// array.
  /// @return A reverse iterator pointing to one past the first element in the
  /// array.
  [[nodiscard]] constexpr reverse_iterator rend() noexcept {
    return reverse_iterator(begin());
  }

  /// Returns a reverse iterator pointing to one past the first element in the
  /// array.
  /// @return A reverse iterator pointing to one past the first element in the
  /// array.
  [[nodiscard]] constexpr const_reverse_iterator rend() const noexcept {
    return const_reverse_iterator(begin());
  }

  /// Returns an iterator pointing to the first element in the array.
  /// @return An iterator pointing to the first element in the array.
  [[nodiscard]] constexpr const_iterator cbegin() const noexcept {
    return const_iterator(data());
  }

  /// Returns an iterator pointing to the past-the-end element in the array.
  /// @return A const_iterator pointing to the past-the-end element in the
  /// array.
  [[nodiscard]] constexpr const_iterator cend() const noexcept {
    return const_iterator(data() + Nm);
  }

  /// Returns a constant reverse iterator pointing to the last element in the
  /// array.
  /// @return A const_reverse_iterator pointing to the last element in the
  /// array.
  [[nodiscard]] constexpr const_reverse_iterator crbegin() const noexcept {
    return const_reverse_iterator(end());
  }

  /// Returns a constant reverse iterator pointing to one past the last element
  /// in the array.
  /// @return A const_reverse_iterator pointing to one past the last element in
  /// the array.
  [[nodiscard]] constexpr const_reverse_iterator crend() const noexcept {
    return const_reverse_iterator(begin());
  }

  /// Returns the number of elements in the array.
  /// @return The number of elements in the array.
  [[nodiscard]] constexpr size_type size() const noexcept { return Nm; }

  /// Returns the maximum possible number of elements that the array can hold.
  /// Since this is a fixed-size array, max_size() returns the same value as
  /// size().
  /// @return The maximum number of elements in the array.
  [[nodiscard]] constexpr size_type max_size() const noexcept { return Nm; }

  /// Checks if the array is empty.
  /// @return True if the array contains no elements, false otherwise.
  [[nodiscard]] constexpr bool empty() const noexcept {
    return size() == static_cast<size_type>(0);
  }

  // Element access.

  /// Returns a reference to the element at specified position in the array.
  /// No bounds checking is performed, so using an index that is out of range
  /// results in undefined behavior.
  /// @param index The zero-based index of the desired element.
  /// @return A reference to the element at the specified position in the array.
  [[nodiscard]] constexpr reference operator[](size_type index) noexcept {
    return internalArray[index];
  }

  /// Returns a constant reference to the element at specified position in the
  /// array. No bounds checking is performed, so using an index that is out of
  /// range results in undefined behavior.
  /// @param index The zero-based index of the desired element.
  /// @return A const_reference to the element at the specified position in the
  /// array.
  [[nodiscard]] constexpr const_reference
  operator[](size_type index) const noexcept {
    return internalArray[index];
  }

  /// Returns a reference to the element at specified position in the array with
  /// bounds checking. If the index is out of range, an std::out_of_range
  /// exception is thrown. Otherwise, a reference to the element at the
  /// specified position is returned.
  /// @param index The zero-based index of the desired element.
  /// @return A reference to the element at the specified position in the array.
  /// @throws std::out_of_range if the given index is out of range.
  constexpr reference at(size_type index) {
    if (index >= Nm) {
      throw std::out_of_range{"Array::at: index (which is " +
                              std::to_string(index) + ") >= Nm (which is " +
                              std::to_string(Nm) + ")"};
    }
    return internalArray[index];
  }

  /// Returns a constant reference to the element at specified position in the
  /// array with bounds checking. If the index is out of range, an
  /// std::out_of_range exception is thrown. Otherwise, a constant reference to
  /// the element at the specified position is returned.
  /// @param index The zero-based index of the desired element.
  /// @return A const_reference to the element at the specified position in the
  /// array.
  /// @throws std::out_of_range if the given index is out of range.
  constexpr const_reference at(size_type index) const {
    // Result of conditional expression must be an lvalue so use
    // boolean ? lvalue : (throw-expr, lvalue)
    return index < Nm ? internalArray[index]
                      : (throw std::out_of_range{"Array::at: index (which is " +
                                                 std::to_string(index) +
                                                 ") >= Nm (which is " +
                                                 std::to_string(Nm) + ")"},
                         internalArray[0]);
  }

  /// Returns a reference to the first element in the array.
  /// This function assumes that the array is non-empty, and does not perform
  /// any bounds checking.
  /// @return A reference to the first element in the array.
  [[nodiscard]] constexpr reference front() noexcept {
    return internalArray[static_cast<size_type>(0)];
  }

  /// Returns a constant reference to the first element in the array.
  /// This function assumes that the array is non-empty, and does not perform
  /// any bounds checking.
  /// @return A const_reference to the first element in the array.
  [[nodiscard]] constexpr const_reference front() const noexcept {
    return internalArray[static_cast<size_type>(0)];
  }

  /// Returns a reference to the last element in the array.
  /// This function assumes that the array is non-empty, and does not perform
  /// any bounds checking.
  /// @return A reference to the last element in the array.
  [[nodiscard]] constexpr reference back() noexcept {
    return internalArray[Nm - static_cast<size_type>(1)];
  }

  /// Returns a constant reference to the last element in the array.
  /// This function assumes that the array is non-empty, and does not perform
  /// any bounds checking.
  /// @return A const_reference to the last element in the array.
  [[nodiscard]] constexpr const_reference back() const noexcept {
    return internalArray[Nm - static_cast<size_type>(1)];
  }

  /// Returns a pointer to the first element in the array.
  /// This function allows direct access to the underlying data of the array,
  /// and can be used for interoperability with C-style APIs or for performance
  /// reasons.
  /// @return A pointer to the first element in the array.
  [[nodiscard]] constexpr pointer data() noexcept {
    return static_cast<pointer>(internalArray);
  }

  /// Returns a const pointer to the first element in the array.
  /// This function allows direct access to the underlying data of the array,
  /// and can be used when passing the array to C-style APIs or when writing
  /// performance-critical code.
  /// @return A const pointer to the first element in the array.
  [[nodiscard]] constexpr const_pointer data() const noexcept {
    return static_cast<const_pointer>(internalArray);
  }

  // Array comparisons.

  /// Checks if two arrays are equal.
  ///  @param lhs A reference to the first array to compare.
  ///  @param rhs A reference to the second array to compare.
  ///  @return True if the arrays have the same number of elements and all
  ///  corresponding elements are equal, false otherwise.
  [[nodiscard]] constexpr friend bool operator==(const Array &lhs,
                                                 const Array &rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin());
  }

  /// Checks if two arrays are not equal.
  /// @param lhs A reference to the first array to compare.
  /// @param rhs A reference to the second array to compare.
  /// @return True if the arrays have a different number of elements or any
  /// corresponding elements are not equal, false otherwise.
  [[nodiscard]] constexpr friend bool operator!=(const Array &lhs,
                                                 const Array &rhs) {
    return !(lhs == rhs);
  }

  /// Checks if one array is lexicographically less than another array.
  /// @param lhs A reference to the first array to compare.
  /// @param rhs A reference to the second array to compare.
  /// @return True if the first array is lexicographically less than the second
  /// array, false otherwise.
  [[nodiscard]] constexpr friend bool operator<(const Array &lhs,
                                                const Array &rhs) {
    return std::lexicographical_compare(lhs.begin(), lhs.end(), rhs.begin(),
                                        rhs.end());
  }

  /// Checks if one array is lexicographically greater than another array.
  /// @param lhs A reference to the first array to compare.
  /// @param rhs A reference to the second array to compare.
  /// @return True if the first array is lexicographically greater than the
  /// second array, false otherwise.
  [[nodiscard]] constexpr friend bool operator>(const Array &lhs,
                                                const Array &rhs) {
    return rhs < lhs;
  }

  /// Checks if one array is lexicographically less than or equal to another
  /// array. This function compares two arrays element-wise using a
  /// lexicographical comparison, and returns true if the first array is less
  /// than or equal to the second array.
  /// @param lhs A reference to the first array to compare.
  /// @param rhs A reference to the second array to compare.
  /// @return True if the first array is lexicographically less than or equal to
  /// the second array, false otherwise.
  [[nodiscard]] constexpr friend bool operator<=(const Array &lhs,
                                                 const Array &rhs) {
    return !(lhs > rhs);
  }

  /// Checks if one array is lexicographically greater than or equal to another
  /// array. This function compares two arrays element-wise using a
  /// lexicographical comparison, and returns true if the first array is greater
  /// than or equal to the second array.
  /// @param lhs A reference to the first array to compare.
  /// @param rhs A reference to the second array to compare.
  /// @return True if the first array is lexicographically greater than or equal
  /// to the second array, false otherwise.

  [[nodiscard]] constexpr friend bool operator>=(const Array &lhs,
                                                 const Array &rhs) {
    return !(lhs < rhs);
  }
};

/// Deduction guide for constructing an Array from a list of elements.
/// This deduction guide allows us to implicitly convert a list of elements into
/// an Array without specifying its type and size explicitly.
/// @tparam Tp The type of the elements in the array.
/// @tparam Args A variadic template pack containing the initial values for the
/// array elements.
/// @return An Array object of type Tp initialized with the given values.
template <class Tp, class... Args,
          class = std::enable_if_t<(std::is_same_v<Tp, Args> && ...), void>>
Array(Tp, Args...) -> Array<Tp, 1 + sizeof...(Args)>;

} // namespace Xsmp

#endif // XSMP_ARRAY_H_
