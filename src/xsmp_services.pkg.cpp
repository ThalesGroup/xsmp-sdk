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
// File Name    : xsmp_services.pkg.cpp
// -----------------------------------------------------------------------------
/// @file xsmp_services.pkg.cpp

// -------------------------------------------------------------------------------
// --------------------------------- Includes ----------------------------------
// -------------------------------------------------------------------------------

#include <xsmp_services.h>

#ifdef  WIN32
#define DLL_EXPORT __declspec(dllexport) // %RELAX<mconst> Visual Studio requires a define
#else
#define DLL_EXPORT
#endif

// -----------------------------------------------------------------------------
// -------------------------- Initialise Function ------------------------------
// -----------------------------------------------------------------------------

extern "C" {

/// Global Initialise function of Package xsmp_services.
/// @param simulator Simulator for registration of factories.
/// @param typeRegistry Type Registry for registration of types.
/// @return True if initialisation was successful, false otherwise.
DLL_EXPORT bool Initialise(::Smp::ISimulator *simulator,
        ::Smp::Publication::ITypeRegistry *typeRegistry) {
    return Initialise_xsmp_services(simulator, typeRegistry);
}
}

// -----------------------------------------------------------------------------
// ---------------------------- Finalise Function ------------------------------
// -----------------------------------------------------------------------------

extern "C" {
/// Global Finalise function of Package xsmp_services.
/// @return True if finalisation was successful, false otherwise.
DLL_EXPORT bool Finalise(::Smp::ISimulator *simulator) {
    return Finalise_xsmp_services(simulator);
}
}
