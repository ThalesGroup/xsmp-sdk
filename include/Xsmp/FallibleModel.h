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

#ifndef XSMP_FALLIBLEMODEL_H_
#define XSMP_FALLIBLEMODEL_H_

#include <Smp/IFailure.h>
#include <Smp/IFallibleModel.h>
#include <Smp/PrimitiveTypes.h>
#include <Xsmp/Collection.h>

namespace Xsmp {
namespace detail {
class Failure;
} // namespace detail

class FallibleModel : public virtual ::Smp::IFallibleModel {
public:
  FallibleModel();
  Smp::Bool IsFailed() const override;
  const Smp::FailureCollection *GetFailures() const override;
  Smp::IFailure *GetFailure(Smp::String8 name) const override;

protected:
  inline void AddFailure(::Smp::IFailure *failure) { _failures.Add(failure); }
  friend class ::Xsmp::detail::Failure;

private:
  ::Xsmp::Collection<::Smp::IFailure> _failures;
};

} // namespace Xsmp

#endif // XSMP_FALLIBLEMODEL_H_
