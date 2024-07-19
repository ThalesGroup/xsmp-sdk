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

#ifndef XSMP_ENTRYPOINT_H_
#define XSMP_ENTRYPOINT_H_

#include <Smp/IEntryPoint.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/cstring.h>
#include <functional>

/// XSMP standard types and interfaces.
namespace Xsmp {

class EntryPointPublisher;

/// @class EntryPoint
/// XSMP implementation of ::Smp::IEntryPoint.
class EntryPoint final : public ::Smp::IEntryPoint {
public:
  /// Constructs a new entry point object with the specified name,
  /// description, parent, and callback.
  /// @param name The name of the entry point.
  /// @param description The description of the entry point.
  /// @param parent The parent object of the entry point.
  /// @param callback A function object representing the action to be executed
  /// when this entry point is invoked.
  EntryPoint(::Smp::String8 name, ::Smp::String8 description,
             ::Xsmp::EntryPointPublisher *parent,
             std::function<void()> &&callback);

  /// Constructs a new entry point object with the specified name,
  /// description, parent, and callback.
  /// @param name The name of the entry point.
  /// @param description The description of the entry point.
  /// @param parent The parent object of the entry point.
  /// @param callback A function object representing the action to be executed
  /// when this entry point is invoked.
  EntryPoint(::Smp::String8 name, ::Smp::String8 description,
             ::Smp::IObject *parent, std::function<void()> &&callback);

  /// Virtual destructor to release memory.
  ~EntryPoint() noexcept override = default;

  /// Return the name of the entry point.
  /// @return  Name of entry point.
  ::Smp::String8 GetName() const override;

  /// Return the description of the entry point.
  /// Descriptions are optional and may be empty.
  /// @return  Description of entry point.
  ::Smp::String8 GetDescription() const override;

  /// Returns the parent object of the entry point.
  /// @return  Parent object of entry point or nullptr if entry point has no
  /// parent.
  ::Smp::IObject *GetParent() const override;

  /// Execute the entry point's action.
  void Execute() const override;

  /// Execute the entry point's action.
  inline void operator()() const { std::invoke(_callback); }

private:
  ::Xsmp::cstring _name;
  ::Xsmp::cstring _description;
  ::Smp::IObject *_parent;
  std::function<void()> _callback;
};

} // namespace Xsmp

#endif // XSMP_ENTRYPOINT_H_
