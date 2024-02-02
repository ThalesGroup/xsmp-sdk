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

#ifndef PYTHON_SMP_IOBJECT_H_
#define PYTHON_SMP_IOBJECT_H_

#include <python/ecss_smp.h>
#include <Smp/AnySimple.h>
#include <Smp/IProperty.h>
#include <Smp/ISimpleField.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IType.h>
#include <Xsmp/Helper.h>

static bool SetValue(::Smp::IObject &self, ::Smp::String8 name,
        const py::handle &value) {

    if (auto *child = ::Xsmp::Helper::Resolve(&self, name)) {

        try {
            if (child == value.cast<::Smp::IObject*>()) {
                return true;
            }
        }
        catch (py::cast_error&) {
            //ignore
        }
        if (auto *property = dynamic_cast<::Smp::IProperty*>(child)) {
            property->SetValue(
                    convert(value,
                            property->GetType()->GetPrimitiveTypeKind()));
            return true;
        }
        if (auto *simpleField = dynamic_cast<::Smp::ISimpleField*>(child)) {
            simpleField->SetValue(
                    convert(value, simpleField->GetPrimitiveTypeKind()));
            return true;
        }
        //todo handle arrays and structs
    }
    return false;
}

inline void RegisterIObject(const py::module_ &m) {
    py::class_<::Smp::IObject>(m, "IObject")

    .def("__getattr__", [](::Smp::IObject &self, ::Smp::String8 name) {
        if (auto *child = ::Xsmp::Helper::Resolve(&self, name)) {
            return child;
        }
        throw py::attribute_error(name);
    },py::arg("name"), py::return_value_policy::reference)

    .def("__setattr__",
            [](::Smp::IObject &self, ::Smp::String8 name,
                    const py::handle &value) {
                if (!SetValue(self, name, value))
                    throw py::attribute_error(name);
            },py::arg("name"), py::arg("value"))

    .def("__getitem__", [](::Smp::IObject &self, ::Smp::String8 name) {
        if (auto *child = ::Xsmp::Helper::Resolve(&self, name)) {
            return child;
        }
        throw py::key_error(name);
    },py::arg("name"), py::return_value_policy::reference)

    .def("__setitem__",
            [](::Smp::IObject &self, ::Smp::String8 name,
                    const py::handle &value) {
                if (!SetValue(self, name, value))
                    throw py::key_error(name);
            },py::arg("name"), py::arg("value"))

    .def("GetName", &::Smp::IObject::GetName, "Return the name of the object.")

    .def("GetDescription", &::Smp::IObject::GetDescription,
            "Return the description of the object.")

    .def("GetParent", &::Smp::IObject::GetParent,
            "Returns the parent object of the object.",
            py::return_value_policy::reference)

    .def("__repr__",
            [](const ::Smp::IObject &self) {
                return "<" + ::Xsmp::Helper::GetPath(&self) + ": "
                        + ::Xsmp::Helper::TypeName(&self) + ">";
            })

    .doc() =
            R"(This interface is the base interface for almost all other SMP interfaces. While most interfaces derive from IComponent, which itself is derived from IObject, some objects (including IField, IFailure, IEntryPoint, IEventSink, IEventSource, IContainer and IReference) are directly derived from IObject.
@remarks The  methods of this interface ensure that all SMP objects can be shown with a name, with an optional description, and under their parent object. Only the simulator itself is expected not to have a parent object, as it is the top-level object of all other objects.)";
}

#endif // PYTHON_SMP_IOBJECT_H_
