// Copyright 2026 THALES ALENIA SPACE FRANCE. All rights reserved.
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

#ifndef XSMP_SIMULATORL2_LOADER_H_
#define XSMP_SIMULATORL2_LOADER_H_

#include <Smp/PrimitiveTypes.h>

#include <memory>

namespace Smp {
class IComponent;
class IObject;
} // namespace Smp

namespace Xsmp {
class Simulator;
}

namespace Xsmp::L2 {

/// Owns all Level 2 document state attached to one simulator.
///
/// The implementation is deliberately hidden from Simulator.h: XML parser
/// types and the representation of deferred actions are private details of the
/// simulator shared library.
class Loader final {
public:
  explicit Loader(::Xsmp::Simulator &simulator);
  ~Loader();

  Loader(const Loader &) = delete;
  Loader &operator=(const Loader &) = delete;

  void LoadAssembly(::Smp::String8 assemblyPath, ::Smp::String8 parentPath,
                    ::Smp::String8 containerName,
                    ::Smp::String8 rootInstanceName);
  void LoadLinkBase(::Smp::String8 linkBasePath, ::Smp::String8 parentPath);
  void LoadSchedule(::Smp::String8 schedulePath);
  void LoadConfiguration(::Smp::String8 configurationPath,
                         ::Smp::String8 parentPath);

  /// Attempt every deferred action once, retaining unresolved actions in
  /// their original order. This is called before each component Configure().
  void RetryDeferred();

  /// Attempt one last pass and raise InvalidFile if anything is unresolved.
  void ResolveOrThrow();

  /// Detach every Schedule callback registered in simulator services.
  ///
  /// This must be called while those services are still alive. The operation
  /// is idempotent so that simulator destruction can safely invoke it from its
  /// unconditional teardown path.
  void Shutdown() noexcept;

private:
  class Impl;
  std::unique_ptr<Impl> _impl;
};

} // namespace Xsmp::L2

#endif // XSMP_SIMULATORL2_LOADER_H_
