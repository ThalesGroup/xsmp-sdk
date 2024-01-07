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

#ifndef PYTHON_SMP_ISIMPLEFIELD_H_
#define PYTHON_SMP_ISIMPLEFIELD_H_


#include <python/ecss_smp.h>
#include <Smp/AnySimple.h>
#include <Smp/ISimpleField.h>
#include <Smp/Publication/IType.h>
#include <Xsmp/Helper.h>
#include <sstream>
#include <string>

inline void RegisterISimpleField(const py::module_ &m) {
    py::class_<::Smp::ISimpleField, ::Smp::IField>(m, "ISimpleField",
            py::multiple_inheritance())

    .def("GetPrimitiveTypeKind", &::Smp::ISimpleField::GetPrimitiveTypeKind,
            "Get primitive type kind that this simple field uses.")

    .def("GetValue", [](const ::Smp::ISimpleField &self) {
        return convert(self.GetValue());
    },
    "Get the value of the simple field.")

    .def("SetValue", [](::Smp::ISimpleField &self, const py::handle &value) {
        self.SetValue(convert(value, self.GetType()->GetPrimitiveTypeKind()));
    },
    "Set the value of the simple field.", py::arg { "value" })

    .def("__bool__", [](const ::Smp::ISimpleField &self) {
        return convert(self.GetValue()).cast<bool>();
    })

    .def("__eq__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()).equal(other);
            })

    .def("__nq__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()).not_equal(other);
            })

    .def("__lt__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) < other;
            })

    .def("__le__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) <= other;
            })

    .def("__gt__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) > other;
            })

    .def("__ge__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) >= other;
            })

    .def("__add__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) + other;
            })

    .def("__sub__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) - other;
            })

    .def("__mul__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) * other;
            })

    .def("__truediv__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) / other;
            })

    .def("__lshift__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) << other;
            })

    .def("__rshift__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) >> other;
            })

    .def("__and__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) & other;
            })

    .def("__xor__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) ^ other;
            })

    .def("__or__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return convert(self.GetValue()) | other;
            })

    .def("__radd__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return other + convert(self.GetValue());
            })

    .def("__rsub__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return other - convert(self.GetValue());
            })

    .def("__rmul__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return other * convert(self.GetValue());
            })

    .def("__rtruediv__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return other / convert(self.GetValue());
            })

    .def("__rlshift__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return other << convert(self.GetValue());
            })

    .def("__rrshift__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return other >> convert(self.GetValue());
            })

    .def("__rand__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return other & convert(self.GetValue());
            })

    .def("__rxor__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return other ^ convert(self.GetValue());
            })

    .def("__ror__",
            [](const ::Smp::ISimpleField &self, const py::handle &other) {
                return other | convert(self.GetValue());
            })

    //////////////////////////////

    .def("__neg__", [](const ::Smp::ISimpleField &self) {
        return -convert(self.GetValue());
    })

    .def("__pos__", [](const ::Smp::ISimpleField &self) {
        return convert(self.GetValue());
    })

    .def("__invert__", [](const ::Smp::ISimpleField &self) {
        return ~convert(self.GetValue());
    })

    .def("__repr__", [](const ::Smp::ISimpleField &self) {
        std::stringstream ss;
        ss << "<" << ::Xsmp::Helper::GetPath(&self) << ": " << self.GetType()->GetName()<< " = " <<self.GetValue()<<">";
        return ss.str();

    })

    .doc() = "Interface of a field of simple type.";
}

#endif // PYTHON_SMP_ISIMPLEFIELD_H_
