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
#include <Xsmp/StorageWriter.h>
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
std::ofstream createOutputStream(::Smp::String8 path, ::Smp::String8 filename,
                                 const ::Smp::IObject *object) {
  if (!fs::is_directory(path ? path : "")) {
    fs::create_directories(path ? path : "");
  }
  auto fullPath = fs::path(path ? path : "") / (filename ? filename : "");
  std::ofstream outputStream{fullPath, std::ios::binary};

  if (!outputStream.good()) {
    ::Xsmp::Exception::throwCannotStore(object, "Cannot open file: " +
                                                    fullPath.string());
  }
  return outputStream;
}
} // namespace

StorageWriter::StorageWriter(::Smp::String8 path, ::Smp::String8 filename,
                             const ::Smp::IObject *object)
    : _path(path ? path : ""), _filename(filename ? filename : ""),
      _object{object}, _os{createOutputStream(path, filename, object)} {}

void StorageWriter::Store(void *address, ::Smp::UInt64 size) {
  _os.write(static_cast<char *>(address), static_cast<std::streamsize>(size));
  if (_os.bad()) {
    ::Xsmp::Exception::throwCannotStore(_object,
                                        "Writing error on output operation");
  }
  if (_os.fail()) {
    ::Xsmp::Exception::throwCannotStore(_object,
                                        "Logical error on output operation");
  }
}

::Smp::String8 StorageWriter::GetStateVectorFileName() const {
  return _filename.c_str();
}

::Smp::String8 StorageWriter::GetStateVectorFilePath() const { return _path.c_str(); }

} // namespace Xsmp
