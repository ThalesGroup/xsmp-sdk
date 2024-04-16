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

#ifndef XSMP_PERSIST_STDPAIR_H_
#define XSMP_PERSIST_STDPAIR_H_

#include <Xsmp/Persist.h>
#include <utility>

namespace Xsmp::Persist {

template <typename K, typename V> struct Helper<std::pair<K, V>> {
  static void Store(const ::Smp::ISimulator *simulator,
                    ::Smp::IStorageWriter *writer,
                    const std::pair<K, V> &value) {
    ::Xsmp::Persist::Store(simulator, writer, value.first, value.second);
  }

  static void Restore(const ::Smp::ISimulator *simulator,
                      ::Smp::IStorageReader *reader, std::pair<K, V> &value) {
    ::Xsmp::Persist::Restore(simulator, reader, value.first, value.second);
  }
};

} // namespace Xsmp::Persist

#endif // XSMP_PERSIST_STDPAIR_H_
