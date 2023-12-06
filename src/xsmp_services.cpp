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

// -----------------------------------------------------------------------------
// File Name    : xsmp_services.cpp
// -----------------------------------------------------------------------------
/// @file xsmp_services.cpp

// ----------------------------------------------------------------------------
// ---------------------------- Include Header Files --------------------
// ----------------------------------------------------------------------------

#include <Smp/ISimulator.h>
#include <xsmp_event_manager.h>
#include <xsmp_link_registry.h>
#include <xsmp_logger.h>
#include <xsmp_resolver.h>
#include <xsmp_scheduler.h>
#include <xsmp_time_keeper.h>
#include "xsmp_services.h"
#include <unordered_set>

// ----------------------------------------------------------------------------------
// ----------------------------- Global variables ------------------------------
// ----------------------------------------------------------------------------------

namespace {
/// Simulators set.
std::unordered_set<::Smp::ISimulator*> simulators { };
} // namespace

// --------------------------------------------------------------------------------
// --------------------------- Initialise Function -----------------------------
// --------------------------------------------------------------------------------

extern "C" {
/// Initialise Package xsmp_services.
/// @param simulator Simulator for registration of factories.
/// @param typeRegistry Type Registry for registration of types.
/// @return True if initialisation was successful, false otherwise.
bool Initialise_xsmp_services(::Smp::ISimulator *simulator,
        [[maybe_unused]] ::Smp::Publication::ITypeRegistry *typeRegistry) {
    // check simulator validity
    if (!simulator) {
        return false;
    }
    // avoid double initialisation
    else if (!::simulators.emplace(simulator).second) {
        return true;
    }

    auto initialised = Initialise_xsmp_logger(simulator, typeRegistry);
    initialised &= Initialise_xsmp_time_keeper(simulator, typeRegistry);
    initialised &= Initialise_xsmp_resolver(simulator, typeRegistry);
    initialised &= Initialise_xsmp_event_manager(simulator, typeRegistry);
    initialised &= Initialise_xsmp_link_registry(simulator, typeRegistry);
    initialised &= Initialise_xsmp_scheduler(simulator, typeRegistry);

    return initialised;
}

}

// ---------------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// ---------------------------------------------------------------------------------

extern "C" {
/// Finalise Package xsmp_services.
/// @param simulator Simulator.
/// @return True if finalisation was successful, false otherwise.
bool Finalise_xsmp_services(::Smp::ISimulator *simulator) {
    // backward compatibility
    if (!simulator) {
        ::simulators.clear();
    }
    // avoid double finalisation
    else if (!::simulators.erase(simulator)) {
        return true;
    }

    auto finalised = Finalise_xsmp_scheduler(simulator);
    finalised &= Finalise_xsmp_link_registry(simulator);
    finalised &= Finalise_xsmp_event_manager(simulator);
    finalised &= Finalise_xsmp_resolver(simulator);
    finalised &= Finalise_xsmp_time_keeper(simulator);
    finalised &= Finalise_xsmp_logger(simulator);

    return finalised;
}
}
