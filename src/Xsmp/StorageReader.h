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

#ifndef XSMP_STORAGEREADER_H_
#define XSMP_STORAGEREADER_H_

#include <Smp/IStorageReader.h>
#include <Smp/PrimitiveTypes.h>
#include <fstream>
#include <string>

namespace Smp {
class IObject;
} // namespace Smp

namespace Xsmp {

class StorageReader : public ::Smp::IStorageReader {
public:
  StorageReader(::Smp::String8 path, ::Smp::String8 filename,
                const ::Smp::IObject *object = nullptr);
  ~StorageReader() noexcept override = default;
  StorageReader(const StorageReader &) = delete;
  StorageReader &operator=(const StorageReader &) = delete;

  /// Restore data from storage.
  /// This method reads a memory block of data from the state vector. It
  /// is the object's responsibility to Store a block of the same size to
  /// IStorageWriter on Store.
  /// @param   address Memory address of memory block.
  /// @param   size Size of memory block.
  void Restore(void *address, ::Smp::UInt64 size) override;

  /// Get the state vector file name.
  /// This method returns a full file name, including the path to the
  /// file.
  /// @return  This method returns the full path of the state vector file
  ///          currently loaded by the Storage Reader.
  ::Smp::String8 GetStateVectorFileName() const override;

  /// Get the state vector file path.
  /// This method returns a full file path that shall be used when
  /// reading additional files that correspond to the state vector file
  /// read by the Storage Reader.
  /// @return  This method returns the full path for auxiliary files
  ///          corresponding to the state vector file currently loaded by
  ///          the Storage Reader.
  ::Smp::String8 GetStateVectorFilePath() const override;

private:
  std::string _path;
  std::string _filename;
  const ::Smp::IObject *_object;
  std::ifstream _is;
};

} // namespace Xsmp

#endif // XSMP_STORAGEREADER_H_
