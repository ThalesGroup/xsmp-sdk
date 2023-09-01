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

#ifndef XSMP_PERSIST_STDARRAY_H_
#define XSMP_PERSIST_STDARRAY_H_

#include <Xsmp/Persist.h>
#include <array>
#include <cstddef>

namespace Xsmp::Persist {

template<typename T, std::size_t N>
struct Helper<std::array<T, N>> {
    static void Store(const ::Smp::ISimulator *simulator,
            ::Smp::IStorageWriter *writer, const std::array<T, N> &value) {
        for (auto &e : value)
            ::Xsmp::Persist::Store(simulator, writer, e);
    }

    static void Restore(const ::Smp::ISimulator *simulator,
            ::Smp::IStorageReader *reader, std::array<T, N> &value) {
        for (size_t i = 0; i < N; ++i)
            ::Xsmp::Persist::Restore(simulator, reader, value[i]);
    }
};

} // namespace Xsmp::Persist

#endif // XSMP_PERSIST_STDARRAY_H_
