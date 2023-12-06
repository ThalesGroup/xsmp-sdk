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

#ifndef XSMP_LIBRARYHELPER_H_
#define XSMP_LIBRARYHELPER_H_

#include <string>

namespace Xsmp {

// Create a string with last error message
[[nodiscard]] std::string GetLastError();

/// Load a library with the given libraryName
/// @param libraryName the library name without the platform dependent prefix and suffix
/// - linux: lib<libraryName>.so
/// - macos: lib<libraryName>.dylib
/// - windows: <libraryName>.dll
void* LoadLibrary(const char *libraryName);

/// Close a library handle
void CloseLibrary(void *handle);

[[nodiscard]] void* GetSymbol(void *handle, const char *symbolName);

template<typename T>
[[nodiscard]] inline T GetSymbol(void *handle, const char *symbolName) {
    return reinterpret_cast<T>(GetSymbol(handle, symbolName));
}
} // namespace Xsmp

#endif // XSMP_LIBRARYHELPER_H_
