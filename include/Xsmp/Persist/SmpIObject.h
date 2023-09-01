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

#ifndef XSMP_PERSIST_SMPIOBJECT_H_
#define XSMP_PERSIST_SMPIOBJECT_H_

#include <Smp/ISimulator.h>
#include <Smp/Services/IResolver.h>
#include <Xsmp/Persist.h>
#include <Xsmp/Helper.h>
#include <string>
#include <type_traits>

namespace Xsmp::Persist {

//serialize reference to an IObject
template<typename T>
struct Helper<T*, std::enable_if_t<std::is_base_of_v<::Smp::IObject, T>>> {
    static void Store(const ::Smp::ISimulator *simulator,
            ::Smp::IStorageWriter *writer, T *const&value) {
        ::Xsmp::Persist::Store(simulator, writer,
                ::Xsmp::Helper::GetPath(value));
    }
    static void Restore(const ::Smp::ISimulator *simulator,
            ::Smp::IStorageReader *reader, T *&value) {
        std::string path;
        ::Xsmp::Persist::Restore(simulator, reader, path);
        value = dynamic_cast<T*>(simulator->GetResolver()->ResolveAbsolute(
                path.c_str()));
    }
};

} // namespace Xsmp::Persist

#endif // XSMP_PERSIST_SMPIOBJECT_H_
