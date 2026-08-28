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

/// XSMP standard types and interfaces.
namespace Xsmp {

// Create a string with last error message
[[nodiscard]] std::string GetLastError();

/// Load a library with the given library name and return a handle to it.
/// @param libraryName The name of the library without the platform dependent
/// prefix and suffix.
/// - Linux: lib<libraryName>.so
/// - MacOS: lib<libraryName>.dylib
/// - Windows: <libraryName>.dll
/// @return The handle of the loaded library, or nullptr if the library could
/// not be loaded.
void *LoadLibrary(const char *libraryName);

/// Close an open library handle.
/// @param handle A pointer to the handle of the loaded library that needs to be
/// closed.
void CloseLibrary(void *handle);

/// Retrieve a symbol from an open library handle.
/// @param handle A pointer to the handle of the loaded library.
/// @param symbolName The name of the symbol to retrieve.
/// @return A pointer to the symbol if it was found, or nullptr otherwise.
[[nodiscard]] void *GetSymbol(void *handle, const char *symbolName);

/// Retrieve a symbol from an open library handle and cast it to a specific
/// type.
/// @tparam T The type of the symbol to retrieve.
/// @param handle A pointer to the handle of the loaded library.
/// @param symbolName The name of the symbol to retrieve.
/// @return A pointer to the symbol if it was found, or nullptr otherwise. The
/// pointer is casted to the specified template type T.
template <typename T>
[[nodiscard]] inline T GetSymbol(void *handle, const char *symbolName) {
  return reinterpret_cast<T>(GetSymbol(handle, symbolName));
}
} // namespace Xsmp

#endif // XSMP_LIBRARYHELPER_H_
