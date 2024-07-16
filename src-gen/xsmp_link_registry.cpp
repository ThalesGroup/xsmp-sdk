// Copyright 2023-2024 THALES ALENIA SPACE FRANCE. All rights reserved.
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
// -----------------------------------------------------------------------------
// File Name    : xsmp_link_registry.cpp
// Generated by : XsmpSdkGenerator-1.1.4
// -----------------------------------------------------------------------------
/// @file xsmp_link_registry.cpp
// This file is auto-generated, Do not edit otherwise your changes will be lost

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------
// ----------------------------------------------------------------------------

#include <Xsmp/Factory.h>
#include <Xsmp/Services/XsmpLinkRegistry.h>
#include <unordered_set>
#include <xsmp_link_registry.h>

// ----------------------------------------------------------------------------------
// ----------------------------- Global variables ------------------------------
// ----------------------------------------------------------------------------------

namespace {
/// Simulators set.
std::unordered_set<::Smp::ISimulator *> simulators{};
} // namespace

// --------------------------------------------------------------------------------
// --------------------------- Initialise Function -----------------------------
// --------------------------------------------------------------------------------

extern "C" {
/// Initialise Package xsmp_link_registry.
/// @param simulator Simulator for registration of factories.
/// @param typeRegistry Type Registry for registration of types.
/// @return True if initialisation was successful, false otherwise.
bool Initialise_xsmp_link_registry(
    ::Smp::ISimulator *simulator,
    [[maybe_unused]] ::Smp::Publication::ITypeRegistry *typeRegistry) {
  // check simulator validity
  if (!simulator) {
    return false;
  }
  // avoid double initialisation
  if (!::simulators.emplace(simulator).second) {
    return true;
  }

  // Register Service XsmpLinkRegistry
  simulator->AddService(
      new ::Xsmp::Services::XsmpLinkRegistry("XsmpLinkRegistry", // name
                                             "",                 // description
                                             simulator,          // parent
                                             simulator           // simulator
                                             ));

  return true;
}
}

// ---------------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// ---------------------------------------------------------------------------------

extern "C" {
/// Finalise Package xsmp_link_registry.
/// @return True if finalisation was successful, false otherwise.
bool Finalise_xsmp_link_registry() {
  // avoid double finalisation
  if (::simulators.empty()) {
    return true;
  }
  ::simulators.clear();

  return true;
}
}
