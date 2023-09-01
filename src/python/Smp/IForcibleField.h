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

#include <python/ecss_smp.h>
#include <pybind11/pybind11.h>
#include <Smp/ISimpleField.h>

#include <Smp/IForcibleField.h>
#include <Smp/Publication/IType.h>
#include <Xsmp/Helper.h>
#include <sstream>
#include <string>

inline void RegisterIForcibleField(const py::module_ &m) {
    py::class_<::Smp::IForcibleField, ::Smp::ISimpleField>(m, "IForcibleField",
            py::multiple_inheritance())

    .def("Force", [](::Smp::IForcibleField &self, const py::object &value) {
        self.Force(convert(value, self.GetPrimitiveTypeKind()));

    },py::arg("value"),
            R"(Force field to given value.
This method raises an exception of type InvalidFieldValue if called with an invalid value.)")

    .def("Unforce", &::Smp::IForcibleField::Unforce, "Unforce field.")

    .def("IsForced", &::Smp::IForcibleField::IsForced,
            "Query for the forced state of the field.")

    .def("Freeze", &::Smp::IForcibleField::Freeze,
            "Force field to its current value.")

    .doc() = R"(Interface of a forcible field.)";
}

