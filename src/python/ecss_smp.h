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

#ifndef PYTHON_ECSS_SMP_H_
#define PYTHON_ECSS_SMP_H_

#include <pybind11/pybind11.h>
#include <Smp/AnySimple.h>
#include <Smp/IObject.h>
#include <Smp/PrimitiveTypes.h>
#include <type_traits>
#include <typeinfo>

namespace py = pybind11;

/// Customized hook for IObject
const ::Smp::IObject* IObjectHook(const ::Smp::IObject *src, const std::type_info *&type);

/// Convert an AnySimple to python type
py::object convert(const ::Smp::AnySimple &value);

/// Convert a python type to an AnySimple
::Smp::AnySimple convert(const py::handle &handle,
        ::Smp::PrimitiveTypeKind kind);

/// Get the index with support for negative index (python like)
::Smp::UInt64 GetIndex(::Smp::Int64 index, ::Smp::UInt64 size);

namespace PYBIND11_NAMESPACE {

/// Use a customized type_hook for IObject
template<typename itype>
struct polymorphic_type_hook<itype,
        std::enable_if_t<std::is_base_of_v<Smp::IObject, itype>>> {
    static const ::Smp::IObject* get(const itype *src, const std::type_info *&type) {
        return IObjectHook(src, type);
    }
};
} // namespace PYBIND11_NAMESPACE

#endif // PYTHON_ECSS_SMP_H_
