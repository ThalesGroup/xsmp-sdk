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

#ifndef PYTHON_SMP_ICONTAINER_H_
#define PYTHON_SMP_ICONTAINER_H_

#include <python/ecss_smp.h>
#include <Smp/IComponent.h>
#include <Smp/IContainer.h>
#include <Smp/PrimitiveTypes.h>
#include <string>

inline void RegisterIContainer(const py::module_ &m) {
    py::class_<::Smp::IContainer, ::Smp::IObject>(m, "IContainer",
            py::multiple_inheritance())

    .def("__len__", &::Smp::IContainer::GetCount)

    .def("GetCount", &::Smp::IContainer::GetCount,
            "Query for the number of components in the collection.")

    .def("GetUpper", &::Smp::IContainer::GetUpper,
            R"(Query the maximum number of components in the collection.
A return value of -1 indicates that the collection has no upper limit.)")

    .def("GetLower", &::Smp::IContainer::GetLower,
            R"(Query the minimum number of components in the collection.)")

    .def("__getitem__",
            [](const ::Smp::IContainer &self, ::Smp::Int64 index) {
                if (auto *cmp = self.GetComponents()->at(
                        GetIndex(index,
                                static_cast<::Smp::UInt64>(self.GetCount()))))
                    return cmp;
                throw py::index_error(std::to_string(index));
            },py::arg("index"), py::return_value_policy::reference)

    .def("__getitem__", [](const ::Smp::IContainer &self, ::Smp::String8 name) {
        if (auto *cmp = self.GetComponents()->at(name))
            return cmp;
        throw py::key_error(name);
    },py::arg("name"), py::return_value_policy::reference)

    .def("__delitem__",
            [](::Smp::IContainer &self, ::Smp::Int64 index) {
                if (auto *cmp = self.GetComponents()->at(
                        GetIndex(index,
                                static_cast<::Smp::UInt64>(self.GetCount()))))
                    self.DeleteComponent(cmp);
                else
                    throw py::index_error(std::to_string(index));
            },py::arg("index"))

    .def("__delitem__", [](::Smp::IContainer &self, ::Smp::String8 name) {
        if (auto *cmp = self.GetComponents()->at(name))
            self.DeleteComponent(cmp);
        else
            throw py::key_error(name);
    },py::arg("name"))

    .def("GetComponent", &::Smp::IContainer::GetComponent, py::arg("name"),
            R"(Query for a component contained in the container by name.
The returned component may be null if no child with the given name could be found.)")

    .def("AddComponent", &::Smp::IContainer::AddComponent, py::arg("component"),
            R"(Add a contained component to the container.
This method raises an exception of type ContainerFull if called for a full container, i.e. when the Count has reached the Upper limit.
It raises an exception of type DuplicateName when trying to add a component with a name that is already contained in the container, as this would lead to duplicate names in the container.
This method may raise an exception of type InvalidObjectType when it expects the given component to implement another interface as well.)")

    .def("DeleteComponent", &::Smp::IContainer::DeleteComponent,
            py::arg("component"),
            R"(Delete a contained component from the container, and from memory.
This method raises an exception of type NotContained if called for a component that has not been added to the container before.
If the container contains the minimum number of components, the  method throws a CannotDelete exception.)")

    .def("__iadd__",
            [](::Smp::IContainer &self,
                    ::Smp::IComponent *component) -> ::Smp::IContainer& {
                self.AddComponent(component);
                return self;
            })

    .def("__isub__",
            [](::Smp::IContainer &self,
                    ::Smp::IComponent *component) -> ::Smp::IContainer& {
                self.DeleteComponent(component);
                return self;
            })

    .doc() =
            R"(Interface for a container.
A container allows querying for its children.
Containers are used together with the IComposite interface for composition.)";
}

#endif // PYTHON_SMP_ICONTAINER_H_
