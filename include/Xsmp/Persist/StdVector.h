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

#ifndef XSMP_PERSIST_STDVECTOR_H_
#define XSMP_PERSIST_STDVECTOR_H_

#include <Xsmp/Persist.h>
#include <vector>

namespace Xsmp::Persist {

template<typename K, typename Alloc>
struct Helper<std::vector<K, Alloc>> {
    using size_type = typename std::vector<K, Alloc>::size_type;
    static void Store(const ::Smp::ISimulator *simulator,
            ::Smp::IStorageWriter *writer, const std::vector<K, Alloc> &value) {
        size_type size = value.size();
        ::Xsmp::Persist::Store(simulator, writer, size);
        for (auto &e : value)
            ::Xsmp::Persist::Store(simulator, writer, e);
    }

    static void Restore(const ::Smp::ISimulator *simulator,
            ::Smp::IStorageReader *reader, std::vector<K, Alloc> &value) {
        value.clear();
        size_type size;
        ::Xsmp::Persist::Restore(simulator, reader, size);

        for (size_type i = 0; i < size; ++i) {
            K k;
            ::Xsmp::Persist::Restore(simulator, reader, k);
            value.push_back(k);
        }
    }
};

} // namespace Xsmp::Persist

#endif // XSMP_PERSIST_STDVECTOR_H_
