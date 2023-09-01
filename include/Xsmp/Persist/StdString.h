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

#ifndef XSMP_PERSIST_STDSTRING_H_
#define XSMP_PERSIST_STDSTRING_H_

#include <Xsmp/Persist.h>
#include <string>

namespace Xsmp::Persist {

/// Helper implementation for std::string elements
template<typename CharT, typename Traits, typename Alloc>
struct Helper<std::basic_string<CharT, Traits, Alloc> > {
    using type = std::basic_string<CharT, Traits,Alloc>;
    using size_type = typename type::size_type;
    static void Store(const ::Smp::ISimulator *simulator,
            ::Smp::IStorageWriter *writer, const type &value) {
        size_type size = value.size();
        ::Xsmp::Persist::Store(simulator, writer, size);
        if (size) {
            type v = value;
            writer->Store(v.data(), size);
        }
    }

    static void Restore(const ::Smp::ISimulator *simulator,
            ::Smp::IStorageReader *reader, type &value) {
        size_type size;
        ::Xsmp::Persist::Restore(simulator, reader, size);
        value.resize(size);
        if (size)
            reader->Restore(value.data(), size);
    }
};

} // namespace Xsmp::Persist

#endif // XSMP_PERSIST_STDSTRING_H_
