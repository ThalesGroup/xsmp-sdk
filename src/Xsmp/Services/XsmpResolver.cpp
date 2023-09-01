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

#include <Xsmp/Services/XsmpResolver.h>
#include <Xsmp/Helper.h>

namespace Xsmp::Services {

::Smp::IObject* XsmpResolver::ResolveAbsolute(::Smp::String8 absolutePath) {

    if (absolutePath && absolutePath[0] == '/') // an absolute path must start with '/'
        return ::Xsmp::Helper::Resolve(GetSimulator(), absolutePath);

    return nullptr;
}

::Smp::IObject* XsmpResolver::ResolveRelative(::Smp::String8 relativePath,
        const ::Smp::IComponent *sender) {

    if (relativePath && relativePath[0] != '/') // a relative path cannot start with '/'
        return ::Xsmp::Helper::Resolve(const_cast<::Smp::IComponent*>(sender),
                relativePath);
    return nullptr;
}

} // namespace Xsmp::Services
