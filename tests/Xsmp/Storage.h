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

#ifndef XSMP_STORAGE_H_
#define XSMP_STORAGE_H_

#include <Smp/IStorageReader.h>
#include <Smp/IStorageWriter.h>
#include <Smp/PrimitiveTypes.h>
#include <iostream>
#include <sstream>

namespace Xsmp {

/// Helper to Store/Restore in memory
class Storage final: public ::Smp::IStorageReader, public ::Smp::IStorageWriter {
public:
    ~Storage() noexcept override = default;

    void Restore(void *address, ::Smp::UInt64 size) override;
    void Store(void *address, ::Smp::UInt64 size) override;
    ::Smp::String8 GetStateVectorFileName() const override;
    ::Smp::String8 GetStateVectorFilePath() const override;
private:
    std::stringstream _ss;
};
} // namespace Xsmp

#endif // XSMP_STORAGE_H_
