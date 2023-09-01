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

namespace Xsmp::Persist {

/// Helper to persist a value
/// this default implementation do not compile.
/// A valid implementation must provide folowing methods:
/// - static void Store(const ::Smp::ISimulator *simulator, ::Smp::IStorageWriter *writer, const T &value) {}
/// - static void Restore(const ::Smp::ISimulator *simulator, ::Smp::IStorageReader *reader, T &value) {}
/// @see Xsmp/Persist/<type>.h for standard implementations
template<typename T, typename = void>
struct Helper {
    static void Store(const ::Smp::ISimulator*, const ::Smp::IStorageWriter*,
            T&) {
        static_assert(std::is_void_v<T>,"Unsupported type. Check your serialization code.");
    }
    static void Restore(const ::Smp::ISimulator*, const ::Smp::IStorageReader*,
            T&) {
        static_assert(std::is_void_v<T>,"Unsupported type. Check your serialization code.");
    }
};

/// Helper implementation for arithmetic / enum types
template<typename T>
struct Helper<T, std::enable_if_t<std::is_arithmetic_v<T> || std::is_enum_v<T>>> {
    static void Store(const ::Smp::ISimulator*, ::Smp::IStorageWriter *writer,
            T value) {
        writer->Store(&value, sizeof(value));
    }
    static void Restore(const ::Smp::ISimulator*, ::Smp::IStorageReader *reader,
            T &value) {
        reader->Restore(&value, sizeof(value));
    }
};

///  Store values
template<typename ... Args>
void Store(const ::Smp::ISimulator *simulator, ::Smp::IStorageWriter *writer,
        const Args &... args) {
    (Helper<Args>::Store(simulator, writer, args), ...);
}

/// Restore values
template<typename ...Args>
void Restore(const ::Smp::ISimulator *simulator, ::Smp::IStorageReader *reader,
        Args &... args) {
    (Helper<Args>::Restore(simulator, reader, args),...);
}

/// Helper implementation for raw array types
template<typename T, std::size_t N>
struct Helper<T[N]> {
    static void Store(const ::Smp::ISimulator *simulator,
            ::Smp::IStorageWriter *writer, const T (&value)[N]) {
        for (std::size_t i = 0; i < N; ++i)
            ::Xsmp::Persist::Store(simulator, writer, value[i]);

    }
    static void Restore(const ::Smp::ISimulator *simulator,
            ::Smp::IStorageReader *reader, T (&value)[N]) {
        for (std::size_t i = 0; i < N; ++i)
            ::Xsmp::Persist::Restore(simulator, reader, value[i]);
    }
};

template<typename ... Args>
void Store(const ::Smp::ISimulator *simulator, const ::Smp::IObject*,
        ::Smp::IStorageWriter *writer, const Args &... args) {
    ((void)Store(simulator, writer, typeid(Args).hash_code(), args), ...);
}

template<typename ... Args>
void Restore(const ::Smp::ISimulator *simulator, const ::Smp::IObject *sender,
        ::Smp::IStorageReader *reader, Args &... args) {

    auto restore = [simulator, sender, reader](auto &value) {
        size_t hash;
        Restore(simulator, reader, hash);
        if (hash != typeid(value).hash_code())
            ::Xsmp::Exception::throwCannotRestore(sender, typeid(value).name());
        Restore(simulator, reader, value);
    };
    (restore(args), ...);
}

} // namespace Xsmp::Persist

#endif // XSMP_PERSIST_H_
