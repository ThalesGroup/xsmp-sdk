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

#ifndef XSMP_PERSIST_STDATOMIC_H_
#define XSMP_PERSIST_STDATOMIC_H_

#include <Xsmp/Persist.h>
#include <atomic>

namespace Xsmp::Persist {

/// Helper implementation for std::string elements
template <typename T> struct Helper<std::atomic<T>> {
  static void Store(const ::Smp::ISimulator *simulator,
                    ::Smp::IStorageWriter *writer,
                    const std::atomic<T> &value) {
    T v = value;
    ::Xsmp::Persist::Store(simulator, writer, v);
  }

  static void Restore(const ::Smp::ISimulator *simulator,
                      ::Smp::IStorageReader *reader, std::atomic<T> &value) {
    T v;
    ::Xsmp::Persist::Restore(simulator, reader, v);
    value = v;
  }
};

} // namespace Xsmp::Persist

#endif // XSMP_PERSIST_STDATOMIC_H_
