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

#include <Smp/IContainer.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Composite.h>

namespace Xsmp {

Composite::Composite() : _containers{"ContainersCollection", "", this} {}

const ::Smp::ContainerCollection *Composite::GetContainers() const {
  return &_containers;
}

::Smp::IContainer *Composite::GetContainer(::Smp::String8 name) const {
  return _containers.at(name);
}
} // namespace Xsmp
