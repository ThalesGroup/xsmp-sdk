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

#ifndef PYTHON_SMP_IREFERENCE_H_
#define PYTHON_SMP_IREFERENCE_H_

#include <python/ecss_smp.h>

#include <Smp/IReference.h>
#include <string>

inline void RegisterIReference(const py::module_ &m) {
    py::class_<::Smp::IReference, ::Smp::IObject>(m, "IReference",
            py::multiple_inheritance())

    .def("__len__", &::Smp::IReference::GetCount)

    .def("GetCount", &::Smp::IReference::GetCount,
            "Query for the number of components in the collection.")

    .def("GetUpper", &::Smp::IReference::GetUpper,
            R"(Query the maximum number of components in the collection.
A return value of -1 indicates that the collection has no upper limit.)")

    .def("GetLower", &::Smp::IReference::GetLower,
            R"(Query the minimum number of components in the collection.)")

    .def("__getitem__",
            [](const ::Smp::IReference &self, ::Smp::Int64 index) {
                if (auto *cmp = self.GetComponents()->at(
                        GetIndex(index,
                                static_cast<::Smp::UInt64>(self.GetCount()))))
                    return cmp;
                throw py::index_error(std::to_string(index));
            },py::arg("index"), py::return_value_policy::reference)

    .def("__getitem__", [](const ::Smp::IReference &self, ::Smp::String8 name) {
        if (auto *cmp = self.GetComponents()->at(name))
            return cmp;
        throw py::key_error(name);
    },py::arg("name"), py::return_value_policy::reference)

    .def("__delitem__",
            [](::Smp::IReference &self, ::Smp::Int64 index) {
                if (auto *cmp = self.GetComponents()->at(
                        GetIndex(index,
                                static_cast<::Smp::UInt64>(self.GetCount()))))
                    self.RemoveComponent(cmp);
                else
                    throw py::index_error(std::to_string(index));
            },py::arg("index"))

    .def("__delitem__", [](::Smp::IReference &self, ::Smp::String8 name) {
        if (auto *cmp = self.GetComponents()->at(name))
            self.RemoveComponent(cmp);
        else
            throw py::key_error(name);
    },py::arg("name"))

    .def("GetComponent", &::Smp::IReference::GetComponent, py::arg("name"),
            R"(Query for a referenced component by its name.
The returned component may be null if no component with the given name could be found.
Multiple components with the same name, but with a different parent (and hence path) may end up in a single reference.
In this case, retrieving a component by name is not save, as any of the components that match the name may be returned.)")

    .def("AddComponent", &::Smp::IReference::AddComponent, py::arg("component"),
            R"(Add a referenced component.
This method raises an exception of type ReferenceFull if called for a full reference, i.e. when the Count has reached the Upper limit.
This method may raise an exception of type InvalidObjectType when it expects the given component to implement another interface as well.
Adding a component with a name that already exists in the reference does not throw an exception, although GetComponent() will no longer allow to return both referenced components by name.)")

    .def("RemoveComponent", &::Smp::IReference::RemoveComponent,
            py::arg("component"),
            R"(Remove a referenced component.
This method raises an exception of type NotReferenced if called with a component that is not referenced.
If the number of referenced components is less than or equal to the Lower limit, this method raises an exception of type CannotRemove.)")

    .def("__iadd__",
            [](::Smp::IReference &self,
                    ::Smp::IComponent *component) -> ::Smp::IReference& {
                self.AddComponent(component);
                return self;
            })

    .def("__isub__",
            [](::Smp::IReference &self,
                    ::Smp::IComponent *component) -> ::Smp::IReference& {
                self.RemoveComponent(component);
                return self;
            })

    .doc() =
            R"(Interface for a reference.
A reference allows querying for the referenced components.)";
}

#endif // PYTHON_SMP_IREFERENCE_H_