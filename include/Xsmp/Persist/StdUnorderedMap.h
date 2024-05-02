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

#ifndef XSMP_PERSIST_STDUNORDEREDMAP_H_
#define XSMP_PERSIST_STDUNORDEREDMAP_H_

#include <Xsmp/Persist.h>
#include <unordered_map>

namespace Xsmp::Persist {

/// Helper implementation for std::map elements
template <typename K, typename V, typename Compare>
struct Helper<std::unordered_map<K, V, Compare>> {
  using type = std::unordered_map<K, V, Compare>;
  using size_type = typename type::size_type;
  static void Store(const ::Smp::ISimulator *simulator,
                    ::Smp::IStorageWriter *writer, const type &value) {
    const size_type size = value.size();
    ::Xsmp::Persist::Store(simulator, writer, size);
    for (auto &e : value) {
      K k = e.first;
      ::Xsmp::Persist::Store(simulator, writer, k, e.second);
    }
  }

  static void Restore(const ::Smp::ISimulator *simulator,
                      ::Smp::IStorageReader *reader, type &value) {
    value.clear();
    size_type size;
    ::Xsmp::Persist::Restore(simulator, reader, size);
    for (size_type i = 0; i < size; ++i) {
      K k;
      V v;
      ::Xsmp::Persist::Restore(simulator, reader, k, v);
      value.emplace(k, v);
    }
  }
};

} // namespace Xsmp::Persist

#endif // XSMP_PERSIST_STDUNORDEREDMAP_H_
