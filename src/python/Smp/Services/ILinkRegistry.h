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
#include <Smp/Services/ILinkRegistry.h>

inline void RegisterILinkRegistry(const py::module_ &m) {

    py::class_<::Smp::Services::ILinkRegistry, ::Smp::IService>(m,
            "ILinkRegistry", py::multiple_inheritance())

    .def("AddLink", &::Smp::Services::ILinkRegistry::AddLink, py::arg("source"),
            py::arg("target"),
            R"(Add a link from source component to target component.
This increments the link count between source and target.
This method informs the link registry that a link between two components has been created. The link registry does not create this link, it only gets told about its existence.
This method can be called several times with the same arguments, when a source component has several links to the same target component.)")

    .def("GetLinkCount", &::Smp::Services::ILinkRegistry::GetLinkCount,
            py::arg("source"), py::arg("target"),
            R"(Returns the number of links between source and target.
This number is incremented each time AddLink() is called, and decremented each time that RemoveLink() is called.)")

    .def("RemoveLink", &::Smp::Services::ILinkRegistry::RemoveLink,
            py::arg("source"), py::arg("target"),
            R"(Remove a link between source and target that has been added to the service using AddLink() before.
This decrements the link count between source and target.
This method informs the link registry that a link between two components has been removed. The link registry does not remove this link, it only gets told about the fact that the link no longer exists.
This method can be called several times with the same arguments, when a source component had several links to the same target component.)")

    .def("GetLinkSources", &::Smp::Services::ILinkRegistry::GetLinkSources,
            py::arg("target"),
            R"(Returns a collection of all sources that have a link to the given target.
This method returns the collection of source components for which a link to the given target component has been added to the link registry.)")

    .def("CanRemove", &::Smp::Services::ILinkRegistry::CanRemove,
            py::arg("target"),
            R"(Returns true if all sources linking to the given target can be asked to remove their link(s), false otherwise.
This method checks whether all sources that have a link to the given target implement the optional interface ILinkingComponent.
If so, they can be asked to remove their links. The method returns false if at least one source exists that does not implement the ILinkingComponent interface.)")

    .def("RemoveLinks", &::Smp::Services::ILinkRegistry::RemoveLinks,
            py::arg("target"),
            R"(Removes all links to the given target.
This method calls the RemoveLinks() method of all source components that implement the optional ILinkingComponent interface, so it asks all link sources to remove their links to the given target.)")

    .doc() =
            R"(This interface is implemented by the Link Registry Service.
The link registry maintains a global collection of link counters between components. Link counters can be incremented and decremented, and can be queried for. Further, the link registry supports fetching and removing all links to a given target.)";
}

