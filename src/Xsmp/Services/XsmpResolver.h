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

#ifndef XSMP_SERVICES_XSMPRESOLVER_H_
#define XSMP_SERVICES_XSMPRESOLVER_H_

#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Services/XsmpResolverGen.h>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------
// ----------------------------------------------------------------------------

namespace Xsmp::Services {

class XsmpResolver final: public XsmpResolverGen {
public:
    // ------------------------------------------------------------------------------------
    // -------------------------- Constructors/Destructor --------------------------
    // ------------------------------------------------------------------------------------

    using XsmpResolverGen::XsmpResolverGen;

    /// Virtual destructor to release memory.
    ~XsmpResolver() noexcept override = default;

    /// Resolve reference to an object via absolute path.
    /// An absolute path contains the name of either a Model or
    /// Service, but not the name of the simulator, although the
    /// simulator itself is the top-level object. This allows keeping
    /// names as short as possible, and avoids a dependency on the name
    /// of the simulator itself.
    /// @param   absolutePath Absolute path to object in simulation.
    /// @return  Object identified by path, or null if no object with
    ///          the given path could be found.
    ::Smp::IObject* ResolveAbsolute(::Smp::String8 absolutePath) override;

    /// Resolve reference to an object via relative path.
    /// @param   relativePath Relative path to object in simulation.
    /// @param   sender Component that asks for resolving the reference.
    /// @return  Component identified by path, or null if no component
    ///          with the given path could be found.
    ::Smp::IObject* ResolveRelative(::Smp::String8 relativePath,
            const ::Smp::IComponent *sender) override;
private:
    friend class ::Xsmp::Component::Helper;
};

} // namespace Xsmp::Services

#endif // XSMP_SERVICES_XSMPRESOLVER_H_
