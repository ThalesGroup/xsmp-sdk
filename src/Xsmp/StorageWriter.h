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

#ifndef XSMP_STORAGEWRITER_H_
#define XSMP_STORAGEWRITER_H_

#include <Smp/IStorageWriter.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/cstring.h>
#include <fstream>

namespace Smp {
class IObject;
} // namespace Smp

namespace Xsmp {

class StorageWriter : public ::Smp::IStorageWriter {
public:
  StorageWriter(::Smp::String8 path, ::Smp::String8 filename,
                const ::Smp::IObject *object = nullptr);
  ~StorageWriter() noexcept override = default;
  StorageWriter(const StorageWriter &) = delete;
  StorageWriter &operator=(const StorageWriter &) = delete;

  /// Store data to storage.
  /// This method writes a memory block of data to the state vector. It
  /// is the object's responsibility to Restore a block of the same size
  /// from IStorageReader on Restore.
  /// @param   address Memory address of memory block.
  /// @param   size Size of memory block.
  void Store(void *address, ::Smp::UInt64 size) override;

  /// Get the state vector file name.
  /// This method returns a full file name, including the path to the
  /// file.
  /// @return  This method returns the full path of the state vector file
  ///          currently stored by the Storage Writer.
  ::Smp::String8 GetStateVectorFileName() const override;

  /// Get the state vector file path.
  /// This method returns a full file path that shall be used when
  /// writing additional files that correspond to the state vector file
  /// written by the Storage Writer.
  /// @return  This method returns the full path for auxiliary files
  ///          corresponding to the state vector file currently stored by
  ///          the Storage Writer.
  ::Smp::String8 GetStateVectorFilePath() const override;

private:
  ::Xsmp::cstring _path;
  ::Xsmp::cstring _filename;
  const ::Smp::IObject *_object;
  std::ofstream _os;
};

} // namespace Xsmp

#endif // XSMP_STORAGEWRITER_H_
