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

#ifndef XSMP_ENTRYPOINTPUBLISHER_H_
#define XSMP_ENTRYPOINTPUBLISHER_H_

#include <Smp/IEntryPoint.h>
#include <Smp/IEntryPointPublisher.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Collection.h>

namespace Xsmp {

class EntryPoint;

class EntryPointPublisher : public virtual ::Smp::IEntryPointPublisher {
public:
  EntryPointPublisher();

  const ::Smp::EntryPointCollection *GetEntryPoints() const override;
  ::Smp::IEntryPoint *GetEntryPoint(::Smp::String8 name) const override;

protected:
  inline void AddEntryPoint(::Smp::IEntryPoint *entryPoint) {
    _entrypoints.Add(entryPoint);
  }

private:
  friend class ::Xsmp::EntryPoint;
  ::Xsmp::Collection<::Smp::IEntryPoint> _entrypoints;
};

} // namespace Xsmp
#endif // XSMP_ENTRYPOINTPUBLISHER_H_
