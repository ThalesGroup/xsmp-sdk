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

#include <Xsmp/LibraryHelper.h>
#include <string>

#if (defined(_WIN32) || defined(_WIN64))
#include <windows.h>
#undef LoadLibrary
#else
#include <dlfcn.h>
#endif

namespace Xsmp {
namespace {

/// Get the library file name:
/// - linux: lib<libraryName>.so
/// - macos: lib<libraryName>.dylib
/// - windows: <libraryName>.dll
std::string LibraryFileName(const char *libraryName) {
#if (defined(_WIN32) || defined(_WIN64))
  return std::string(libraryName) + ".dll";
#elif defined(__APPLE__)
  return std::string("lib") + libraryName + ".dylib";
#else
  return std::string("lib") + libraryName + ".so";
#endif
}

#if (defined(_WIN32) || defined(_WIN64))
/// Get the directory of the module holding this code, with its trailing
/// separator. LoadLibrary called with a file name searches the directory of the
/// executable, not the one of the caller: a library deployed next to the SDK,
/// as in the Python wheel, is not found without it.
std::string ModuleDirectory() {
  HMODULE handle = nullptr;
  if (!GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS |
                              GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT,
                          reinterpret_cast<LPCSTR>(&LibraryFileName),
                          &handle)) {
    return {};
  }
  char path[MAX_PATH] = {};
  const DWORD size = GetModuleFileNameA(handle, path, MAX_PATH);
  if (size == 0 || size >= MAX_PATH) {
    return {};
  }
  const std::string result{path, size};
  const auto separator = result.find_last_of("\\/");
  return separator == std::string::npos ? std::string{}
                                        : result.substr(0, separator + 1);
}
#endif

} // namespace

void *LoadLibrary(const char *libraryName) {
  if (!libraryName) {
    return nullptr;
  }
#if (defined(_WIN32) || defined(_WIN64))
  const auto fileName = LibraryFileName(libraryName);
  if (auto *handle = LoadLibraryA(fileName.c_str())) {
    return handle;
  }
  const auto directory = ModuleDirectory();
  if (directory.empty()) {
    return nullptr;
  }
  return LoadLibraryA((directory + fileName).c_str());
#else
  // On MacOs the default is RTLD_GLOBAL
  // On Linux the default is RTLD_LOCAL
  return dlopen(LibraryFileName(libraryName).c_str(), RTLD_NOW);
#endif
}

void CloseLibrary(void *handle) {
  if (!handle) {
    return;
  }
#if (defined(_WIN32) || defined(_WIN64))
  FreeLibrary(static_cast<HMODULE>(handle));
#else
  dlclose(handle);
#endif
}

void *GetSymbol(void *handle, const char *symbolName) {
  if (!symbolName) {
    return nullptr;
  }
#if (defined(_WIN32) || defined(_WIN64))
  return GetProcAddress(static_cast<HMODULE>(handle), symbolName);
#else
  dlerror(); // Clear any existing error
  return dlsym(handle, symbolName);
#endif
}

// Create a string with last error message
std::string GetLastError() {
#if (defined(_WIN32) || defined(_WIN64))
  DWORD error = ::GetLastError();
  if (error) {
    LPVOID lpMsgBuf;
    DWORD bufLen = FormatMessageW(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
            FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL, error, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPWSTR)&lpMsgBuf, 0, NULL);

    if (bufLen) {
      LPCWSTR lpMsgStr = (LPCWSTR)lpMsgBuf;
      int utf8BufferSize =
          WideCharToMultiByte(CP_UTF8, 0, lpMsgStr, -1, NULL, 0, NULL, NULL);
      std::string result;
      if (utf8BufferSize > 0) {
        // utf8BufferSize accounts for the terminating null character
        result.resize(static_cast<std::size_t>(utf8BufferSize));
        WideCharToMultiByte(CP_UTF8, 0, lpMsgStr, -1, &result[0],
                            utf8BufferSize, NULL, NULL);
        result.resize(static_cast<std::size_t>(utf8BufferSize) - 1);
      }
      // the buffer allocated by FormatMessageW must be released on every path
      LocalFree(lpMsgBuf);
      if (!result.empty()) {
        return result;
      }
    }
  }
#else
  if (auto const *error = dlerror()) {
    return error;
  }
#endif
  return {};
}

} // namespace Xsmp
