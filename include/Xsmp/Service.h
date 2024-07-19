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

#ifndef XSMP_SERVICE_H_
#define XSMP_SERVICE_H_

#include <Smp/IService.h>
#include <Xsmp/Component.h>

/// XSMP standard types and interfaces.
namespace Xsmp {

/// @class Service
/// XSMP implementation of ::Smp::IService.
class Service : public Component, public virtual ::Smp::IService {
public:
  using Component::Component;
};

} // namespace Xsmp

#endif // XSMP_SERVICE_H_
