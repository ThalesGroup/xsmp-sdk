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

#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Exception.h>
#include <Xsmp/StorageReader.h>
#include <fstream>
#include <iostream>

#if !defined(__clang__) && defined(__GNUC__) && __GNUC__ <= 7
#include <experimental/filesystem>
namespace fs = std::experimental::filesystem;
#else
#include <filesystem>
namespace fs = std::filesystem;
#endif

namespace Xsmp {
namespace {
std::ifstream createInputStream(::Smp::String8 path, ::Smp::String8 filename,
                                const ::Smp::IObject *object) {

  auto fullPath = fs::path(path ? path : "") / (filename ? filename : "");
  std::ifstream is{fullPath, std::ios::binary};

  if (!is.good()) {
    ::Xsmp::Exception::throwCannotRestore(object, "Cannot open file: " +
                                                      fullPath.string());
  }
  return is;
}
} // namespace

StorageReader::StorageReader(::Smp::String8 path, ::Smp::String8 filename,
                             const ::Smp::IObject *object)
    : _path(path ? path : ""), _filename(filename ? filename : ""),
      _object{object}, _is(createInputStream(path, filename, object)) {}

void StorageReader::Restore(void *address, ::Smp::UInt64 size) {
  _is.read(static_cast<char *>(address), static_cast<std::streamsize>(size));
  if (_is.bad()) {
    ::Xsmp::Exception::throwCannotRestore(_object,
                                          "Read/ error on input operation");
  }
  if (_is.fail()) {
    ::Xsmp::Exception::throwCannotRestore(_object,
                                          "Logical error on input operation");
  }
  if (_is.eof()) {
    ::Xsmp::Exception::throwCannotRestore(
        _object, "End-of-File reached on input operation");
  }
}

::Smp::String8 StorageReader::GetStateVectorFileName() const {
  return _filename.c_str();
}

::Smp::String8 StorageReader::GetStateVectorFilePath() const {
  return _path.c_str();
}

} // namespace Xsmp
