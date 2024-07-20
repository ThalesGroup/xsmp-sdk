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

#ifndef XSMP_PERSIST_H_
#define XSMP_PERSIST_H_

#include <Smp/IStorageReader.h>
#include <Smp/IStorageWriter.h>
#include <Xsmp/Exception.h>
#include <cstddef>
#include <type_traits>
#include <typeinfo>

namespace Smp {
class ISimulator;
} // namespace Smp

/// XSMP standard types and interfaces.
namespace Xsmp::
    /// XSMP persist interfaces.
    Persist {

/// Helper struct that defines how to serialize and deserialize a type `T`. This
/// default implementation will fail at compile time with a static assertion if
/// an unsupported type is encountered. A valid implementation must provide two
/// methods:
/// - `Store`: Writes the value of type T to a storage writer.
/// - `Restore`: Reads and populates a value of type T from a storage reader.
///
/// This struct serves as a fallback when no specialized implementation for a
/// specific type is available. If an unsupported type is encountered, the code
/// will fail at compile time with a static assertion error to notify the user
/// that additional serialization code needs to be implemented for the new type.
///
/// @tparam T The type to be serialized or deserialized.
/// @see Xsmp/Persist/<type>.h for standard implementations
template <typename T, typename = void> struct Helper {
  /// Writes the value of type `T` to a storage writer.
  ///
  /// @param simulator A pointer to the ISimulator object
  /// operations.
  /// @param writer A pointer to the IStorageWriter object where the value will
  /// be written.
  /// @param value Reference to the value that needs to be stored.
  static void Store(const ::Smp::ISimulator *, const ::Smp::IStorageWriter *,
                    T &) {
    static_assert(std::is_void_v<T>,
                  "Unsupported type. Check your serialization code.");
  }

  /// Reads and populates a value of type T from a storage reader.
  ///
  /// @param simulator A pointer to the ISimulator object
  /// @param reader A pointer to the IStorageReader object where the value will
  /// be read from.
  /// @param value Reference to the variable that needs to store the restored
  /// value.
  static void Restore(const ::Smp::ISimulator *, const ::Smp::IStorageReader *,
                      T &) {
    static_assert(std::is_void_v<T>,
                  "Unsupported type. Check your serialization code.");
  }
};

/// Helper implementation for arithmetic / enum types. This specialization of
/// the `Helper` template allows for the storage and restoration of values that
/// are either arithmetic or enumeration types.
template <typename T>
struct Helper<T,
              std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>> {
  static void Store(const ::Smp::ISimulator *, ::Smp::IStorageWriter *writer,
                    T value) {
    writer->Store(&value, sizeof(value));
  }
  static void Restore(const ::Smp::ISimulator *, ::Smp::IStorageReader *reader,
                      T &value) {
    reader->Restore(&value, sizeof(value));
  }
};

/// Store multiple values into a storage writer.
///
/// @tparam Args Variadic template parameter pack containing the types of values
/// to be stored.
/// @param simulator A pointer to the ISimulator object used for persistence
/// operations.
/// @param writer A pointer to the IStorageWriter object where the values will
/// be written.
/// @param args References to the values that need to be stored.
template <typename... Args>
void Store(const ::Smp::ISimulator *simulator, ::Smp::IStorageWriter *writer,
           const Args &...args) {
  (Helper<Args>::Store(simulator, writer, args), ...);
}

/// Restore multiple values from a storage reader.
///
/// @tparam Args Variadic template parameter pack containing the types of values
/// to be restored.
/// @param simulator A pointer to the ISimulator object used for restoration
/// operations.
/// @param reader A pointer to the IStorageReader object where the values will
/// be read from.
/// @param args References to the variables that need to store the restored
/// values.
template <typename... Args>
void Restore(const ::Smp::ISimulator *simulator, ::Smp::IStorageReader *reader,
             Args &...args) {
  (Helper<Args>::Restore(simulator, reader, args), ...);
}

/// Helper implementation for raw array types. This specialization of the
/// `Helper` template allows for the storage and restoration of arrays of a
/// specified type and size.
template <typename T, std::size_t N> struct Helper<T[N]> {
  static void Store(const ::Smp::ISimulator *simulator,
                    ::Smp::IStorageWriter *writer, const T (&value)[N]) {
    for (size_t i = 0; i < N; ++i) {
      ::Xsmp::Persist::Store(simulator, writer, value[i]);
    }
  }
  static void Restore(const ::Smp::ISimulator *simulator,
                      ::Smp::IStorageReader *reader, T (&value)[N]) {
    for (size_t i = 0; i < N; ++i) {
      ::Xsmp::Persist::Restore(simulator, reader, value[i]);
    }
  }
};

/// Store multiple values, along with their type hashes, into a storage writer.
/// This function stores each value along with its corresponding type hash,
/// which can be used for later restoration to ensure that the restored values
/// are of the correct types.
///
/// @tparam Args Variadic template parameter pack containing the types of values
/// to be stored.
/// @param simulator A pointer to the ISimulator object used for persistence
/// operations.
/// @param sender A pointer to the IObject representing the sender. (This
/// parameter is not currently used in this function but may be useful in future
/// revisions.)
/// @param writer A pointer to the IStorageWriter object where the values will
/// be written.
/// @param args References to the values that need to be stored.
template <typename... Args>
void Store(const ::Smp::ISimulator *simulator, const ::Smp::IObject *,
           ::Smp::IStorageWriter *writer, const Args &...args) {
  ((void)Store(simulator, writer, typeid(Args).hash_code(), args), ...);
}

/// Restore multiple values from a storage reader, along with their type hashes.
/// This function restores each value while checking its corresponding type hash
/// to ensure that the restored values are of the correct types. If a mismatch
/// is detected between the expected and actual type hashes, an exception will
/// be thrown.
///
/// @tparam Args Variadic template parameter pack containing the types of values
/// to be restored.
/// @param simulator A pointer to the ISimulator object used for restoration
/// operations.
/// @param sender A pointer to the IObject representing the sender. This is used
/// in case an exception needs to be thrown due to a type mismatch.
/// @param reader A pointer to the IStorageReader object where the values will
/// be read from.
/// @param args References to the variables that need to store the restored
/// values.
template <typename... Args>
void Restore(const ::Smp::ISimulator *simulator, const ::Smp::IObject *sender,
             ::Smp::IStorageReader *reader, Args &...args) {

  auto restore = [simulator, sender, reader](auto &value) {
    size_t hash = 0;
    Restore(simulator, reader, hash);
    if (hash != typeid(value).hash_code()) {
      ::Xsmp::Exception::throwCannotRestore(sender, typeid(value).name());
    }
    Restore(simulator, reader, value);
  };
  (restore(args), ...);
}

} // namespace Xsmp::Persist

#endif // XSMP_PERSIST_H_
