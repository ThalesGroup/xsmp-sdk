// Copyright 2023-2024 THALES ALENIA SPACE FRANCE. All rights reserved.
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

#ifndef PYTHON_SMP_ISIMULATOR_H_
#define PYTHON_SMP_ISIMULATOR_H_

#include <Smp/IAggregate.h>
#include <Smp/IArrayField.h>
#include <Smp/IContainer.h>
#include <Smp/IDataflowField.h>
#include <Smp/IDynamicInvocation.h>
#include <Smp/IEntryPointPublisher.h>
#include <Smp/IEventConsumer.h>
#include <Smp/IEventProvider.h>
#include <Smp/IEventSink.h>
#include <Smp/IEventSource.h>
#include <Smp/IFailure.h>
#include <Smp/IForcibleField.h>
#include <Smp/ILinkingComponent.h>
#include <Smp/IModel.h>
#include <Smp/IOperation.h>
#include <Smp/IParameter.h>
#include <Smp/IProperty.h>
#include <Smp/IReference.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimulator.h>
#include <Smp/IStructureField.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ILinkRegistry.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/IResolver.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Smp/SimulatorStateKind.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/Exception.h>
#include <Xsmp/LibraryHelper.h>
#include <chrono>
#include <fstream>
#include <memory>
#include <python/ecss_smp.h>
#include <string>
#include <thread>

std::unique_ptr<::Smp::ISimulator> createSimulator(::Smp::String8 libraryName,
                                                   ::Smp::String8 factoryName,
                                                   ::Smp::String8 name,
                                                   ::Smp::String8 description) {

  void *handle = ::Xsmp::LoadLibrary(libraryName);

  if (!handle) {
    ::Xsmp::Exception::throwLibraryNotFound(nullptr, libraryName,
                                            ::Xsmp::GetLastError());
  }

  auto *factory =
      ::Xsmp::GetSymbol<::Smp::ISimulator *(*)(::Smp::String8, ::Smp::String8)>(
          handle, factoryName);
  if (!factory) {
    ::Xsmp::Exception::throwInvalidLibrary(nullptr, libraryName,
                                           ::Xsmp::GetLastError());
  }

  return std::unique_ptr<::Smp::ISimulator>((*factory)(name, description));
}

void Run(::Smp::ISimulator &self, double ns, double us, double ms, double s,
         double mn, double h, double d, double w) {
  d += 7 * w;
  h += 24 * d;
  mn += 60 * h;
  s += 60 * mn;
  ms += 1000 * s;
  us += 1000 * ms;
  ns += 1000 * us;
  auto duration = static_cast<::Smp::Duration>(ns);

  if (self.GetState() == ::Smp::SimulatorStateKind::SSK_Standby) {
    ::Xsmp::EntryPoint hold{"hold",
                            "call simulator hold after the specified duration",
                            &self, [&self] { self.Hold(false); }};
    const auto eventId =
        self.GetScheduler()->AddSimulationTimeEvent(&hold, duration);

    // `hold` lives on the stack: the event must be removed from the scheduler
    // on every exit path, otherwise the scheduler keeps a pointer to a
    // destroyed entry point and executes it during a later Run().
    const auto removeHoldEvent = [&self, eventId]() noexcept {
      try {
        self.GetScheduler()->RemoveEvent(eventId);
      } catch (...) {
        // the event has already been executed and removed by the scheduler
      }
    };

    try {
      self.Run();

      while (self.GetState() == ::Smp::SimulatorStateKind::SSK_Executing) {
        if (PyErr_CheckSignals() != 0) {
          self.Hold(true);
          throw py::error_already_set();
        }
        std::this_thread::sleep_for(std::chrono::microseconds(10));
      }
    } catch (...) {
      removeHoldEvent();
      throw;
    }
    removeHoldEvent();
  } else {
    self.Run();
  }
}

constexpr const char *INDENT = "    ";

/// Emit a member that is read as `type` but can be assigned any python value.
///
/// Reading a field or a property yields the corresponding SMP object, while
/// assigning to it goes through IObject.__setattr__ and accepts a plain python
/// value. A bare `name: type` annotation would make every assignment a type
/// error, hence the property/setter pair.
void generateAssignableMember(std::ostream &fs, const std::string &indent,
                              const std::string &name, const std::string &type,
                              ::Smp::String8 description) {
  fs << indent << "@property\n";
  fs << indent << "def " << name << "(self) -> " << type << ":\n";
  if (description && description[0] != 0)
    fs << indent << INDENT << R"(""")" << description << R"(""")"
       << "\n";
  // the dump is a regular module, so a docstring alone is not a valid body
  fs << indent << INDENT << "...\n";
  fs << indent << "@" << name << ".setter\n";
  fs << indent << "def " << name << "(self, value: typing.Any) -> None: ...\n";
}

void generate(std::ostream &fs, const std::string &indent,
              const ::Smp::IObject *obj) {

  std::stringstream bases;

  std::stringstream body;

  if (auto *component = dynamic_cast<const ::Smp::IComponent *>(obj)) {
    if (dynamic_cast<const ::Smp::IModel *>(component))
      bases << "ecss_smp.Smp.IModel, ";
    else if (auto *service = dynamic_cast<const ::Smp::IService *>(component)) {
      if (dynamic_cast<const ::Smp::Services::IEventManager *>(service))
        bases << "ecss_smp.Smp.Services.IEventManager, ";
      else if (dynamic_cast<const ::Smp::Services::ILinkRegistry *>(service))
        bases << "ecss_smp.Smp.Services.ILinkRegistry, ";
      else if (dynamic_cast<const ::Smp::Services::ILogger *>(service))
        bases << "ecss_smp.Smp.Services.ILogger, ";
      else if (dynamic_cast<const ::Smp::Services::IResolver *>(service))
        bases << "ecss_smp.Smp.Services.IResolver, ";
      else if (dynamic_cast<const ::Smp::Services::IScheduler *>(service))
        bases << "ecss_smp.Smp.Services.IScheduler, ";
      else if (dynamic_cast<const ::Smp::Services::ITimeKeeper *>(service))
        bases << "ecss_smp.Smp.Services.ITimeKeeper, ";
      else
        bases << "ecss_smp.Smp.IService, ";
    }

    if (const auto *aggregate =
            dynamic_cast<const ::Smp::IAggregate *>(component)) {
      bases << "ecss_smp.Smp.IAggregate, ";

      if (const auto *references = aggregate->GetReferences())
        for (const auto *reference : *references) {
          body << indent + INDENT << "_" << reference->GetName()
               << ": ecss_smp.Smp.IReference\n";
          body << "\n";
        }
    }

    if (const auto *dynamiqueInvocation =
            dynamic_cast<const ::Smp::IDynamicInvocation *>(component)) {
      bases << "ecss_smp.Smp.IDynamicInvocation, ";

      for (const auto *operation : *dynamiqueInvocation->GetOperations()) {
        body << indent + INDENT << "def " << operation->GetName() << "(self";
        // parameters are converted from/to python values, their SMP type
        // gives no usable python annotation
        for (const auto *parameter : *operation->GetParameters())
          body << ", " << parameter->GetName() << ": typing.Any";
        body << ") -> typing.Any: ...\n";
      }
      for (const auto *property : *dynamiqueInvocation->GetProperties())
        generateAssignableMember(body, indent + INDENT, property->GetName(),
                                 "ecss_smp.Smp.IProperty",
                                 property->GetDescription());
    }
    if (const auto *eventConsumer =
            dynamic_cast<const ::Smp::IEventConsumer *>(component)) {
      bases << "ecss_smp.Smp.IEventConsumer, ";

      if (const auto *eventSinks = eventConsumer->GetEventSinks())
        for (const auto *eventSink : *eventSinks) {
          body << indent + INDENT << eventSink->GetName()
               << ": ecss_smp.Smp.IEventSink\n";
          body << "\n";
        }
    }
    if (const auto *eventProvider =
            dynamic_cast<const ::Smp::IEventProvider *>(component)) {
      bases << "ecss_smp.Smp.IEventProvider, ";

      if (const auto *eventSources = eventProvider->GetEventSources())
        for (const auto *eventSource : *eventSources) {
          body << indent + INDENT << eventSource->GetName()
               << ": ecss_smp.Smp.IEventSource\n";
          body << "\n";
        }
    }
    if (dynamic_cast<const ::Smp::ILinkingComponent *>(component)) {
      bases << "ecss_smp.Smp.ILinkingComponent, ";
    }

    for (const auto *field : *component->GetFields())
      generate(body, indent + INDENT, field);
  }

  if (const auto *composite = dynamic_cast<const ::Smp::IComposite *>(obj)) {
    if (dynamic_cast<const ::Smp::ISimulator *>(obj))
      bases << "ecss_smp.Smp.ISimulator, ";
    else
      bases << "ecss_smp.Smp.IComposite, ";

    if (const auto *containers = composite->GetContainers())
      for (const auto *container : *containers) {
        body << indent + INDENT << "_" << container->GetName()
             << ": ecss_smp.Smp.IContainer\n";
        body << "\n";
        for (const auto *cmp : *container->GetComponents())
          generate(body, indent + INDENT, cmp);
      }
  }
  if (const auto *entryPointPublisher =
          dynamic_cast<const ::Smp::IEntryPointPublisher *>(obj)) {
    bases << "ecss_smp.Smp.IEntryPointPublisher, ";

    for (const auto *entryPoint : *entryPointPublisher->GetEntryPoints())
      body << indent + INDENT << entryPoint->GetName()
           << ": ecss_smp.Smp.IEntryPoint\n";
  }

  if (dynamic_cast<const ::Smp::IPersist *>(obj)) {

    if (dynamic_cast<const ::Smp::IField *>(obj)) {

      if (dynamic_cast<const ::Smp::ISimpleField *>(obj)) {
        if (dynamic_cast<const ::Smp::IForcibleField *>(obj))
          bases << "ecss_smp.Smp.IForcibleField, ";
        else
          bases << "ecss_smp.Smp.ISimpleField, ";
        // field->GetPrimitiveTypeKind()
      } else if (dynamic_cast<const ::Smp::IArrayField *>(obj)) {
        bases << "ecss_smp.Smp.IArrayField, "; // TODO type hint for item type
      } else if (dynamic_cast<const ::Smp::ISimpleArrayField *>(obj)) {
        bases << "ecss_smp.Smp.ISimpleArrayField, "; // TODO type hint for item
                                                     // type
      } else if (auto *structureField =
                     dynamic_cast<const ::Smp::IStructureField *>(obj)) {
        bases << "ecss_smp.Smp.IStructureField, ";
        for (const auto *nestedField : *structureField->GetFields())
          generate(body, indent + INDENT, nestedField);
      }

      if (dynamic_cast<const ::Smp::IDataflowField *>(obj)) {
        bases << "ecss_smp.Smp.IDataflowField, ";
      }
      if (dynamic_cast<const ::Smp::IFailure *>(obj)) {
        bases << "ecss_smp.Smp.IFailure, ";
      }
    } else if (dynamic_cast<const ::Smp::IFailure *>(obj)) {
      bases << "ecss_smp.Smp.IFailure, ";
    } else {
      bases << "ecss_smp.Smp.IPersist, ";
    }
  }

  if (body.str().empty())
    body << indent + INDENT << "pass\n";

  if (dynamic_cast<const ::Smp::ISimulator *>(obj)) {
    fs << indent << "class Simulator(" << bases.str() << "):\n";

    fs << body.str();
    fs << "\n";
  } else {
    fs << indent << "class __" << obj->GetName() << "(" << bases.str()
       << "):\n";

    fs << body.str();
    fs << "\n";
    if (dynamic_cast<const ::Smp::IField *>(obj)) {
      generateAssignableMember(fs, indent, obj->GetName(),
                               std::string("__") + obj->GetName(),
                               obj->GetDescription());
    } else {
      fs << indent << obj->GetName() << ": __" << obj->GetName() << "\n";
      if (obj->GetDescription()[0] != 0)
        fs << indent << R"(""")" << obj->GetDescription() << R"("""
)";
    }
    fs << "\n";
  }
}

void generatePythonTypeHints(const ::Smp::ISimulator &self,
                             const std::string &path) {

  std::ofstream fs{path};

  fs << "import typing\n";
  fs << "\n";
  fs << "import ecss_smp\n";
  fs << "\n";
  fs << "\n";

  generate(fs, "", &self);

  fs << "\n";
  // importing the class would bind `type[Simulator]`, and every instance
  // method would then ask for an explicit `self`; importing this value binds
  // the instance, and its name is the one the test case uses
  fs << "sim: Simulator\n";

  fs.close();
}

inline void RegisterISimulator(py::module_ &m) {
  py::class_<::Smp::ISimulator, ::Smp::IComposite>(m, "ISimulator",
                                                   py::multiple_inheritance())

      .def(
          "Initialise", &::Smp::ISimulator::Initialise,
          R"(This method asks the simulation environment to call all initialisation entry points again.
This method can only be called when in Standby state, and enters Initialising state. After completion, the simulator automatically returns to Standby state.
The entry points will be executed in the order they have been added to the simulator using the AddInitEntryPoint() method.)")

      .def(
          "Publish", &::Smp::ISimulator::Publish,
          R"(This method asks the simulation environment to call the Publish() method of all service and model instances in the component hierarchy which are still in Created state.
This method must only be called when in Building state.)")

      .def(
          "Configure", &::Smp::ISimulator::Configure,
          R"(This method asks the simulation environment to call the Configure() method of all service and model instances which are still in Publishing state.
This method must only be called when in Building state.)")

      .def(
          "Connect", &::Smp::ISimulator::Connect,
          R"(This method informs the simulation environment that the hierarchy of model instances has been configured, and can now be connected to the simulator. Thus, the simulation environment calls the Connect()method of all service and model instances.
In this method, the simulation environment first enters Connecting state and calls the Connect() method of every model in the model hierarchy, then enters Initialising state and calls the initialisation entry points, and finally enters Standby state.
This method must only be called when in Building state.)")

      .def("Run", &::Smp::ISimulator::Run,
           R"(This method changes from Standby to Executing state.
This method must only be called when in Standby state, and enters Executing state.)") // TODO wait until end

      .def(
          "Run", &Run, py::arg("nanoseconds") = 0.,
          py::arg("microseconds") = 0., py::arg("milliseconds") = 0.,
          py::arg("seconds") = 0., py::arg("minutes") = 0.,
          py::arg("hours") = 0., py::arg("days") = 0., py::arg("weeks") = 0.,
          R"(This method changes from Standby to Executing state for the specified duration.
This method must only be called when in Standby state, and enters Executing state.)")

      .def("Hold", &::Smp::ISimulator::Hold, py::arg("immediate"),
           R"(This method changes from Executing to Standby state.
This method must only be called when in Executing state, and enters Standby state.)")

      .def("Store", &::Smp::ISimulator::Store, py::arg("file_name"),
           R"(This method is used to store a state vector to file.
This method must only be called when in Standby state, and enters Storing state. On completion, it automatically returns to Standby state.)")

      .def("Restore", &::Smp::ISimulator::Restore, py::arg("file_name"),
           R"(This method is used to restore a state vector from file.
This method must only be called when in Standby state, and enters Restoring state. On completion, it automatically returns to Standby state.)")

      .def(
          "Reconnect", &::Smp::ISimulator::Reconnect, py::arg("root"),
          R"(This method asks the simulation environment to reconnect the component hierarchy starting at the given root component.
This method must only be called when in Standby state.)")

      .def("Exit", &::Smp::ISimulator::Exit,
           R"(This method is used for a normal termination of a simulation.
This method must only be called when in Standby state, and enters Exiting state.)")

      .def("Abort", &::Smp::ISimulator::Abort,
           R"(This method is used for an abnormal termination of a simulation.
This method can be called from any other state, and enters Aborting state.)")

      .def("GetState", &::Smp::ISimulator::GetState,
           R"(Return the current simulator state.)")

      .def(
          "AddInitEntryPoint", &::Smp::ISimulator::AddInitEntryPoint,
          py::arg("entry_point"),
          R"(This method can be used to add entry points that shall be executed in the Initialising state.
If the simulation is in Building, Connecting or Standby state, it adds the entry point to the list of entry points to be executed once the simulation reaches Initialising state.
If the simulation is not in Building, Connecting or Standby state, then it returns and no action is taken.
This allows components to subscribe to a callback during initialization phase since there are only explicit methods defined for Publish, Configure and Connect. This choice is taken since most
models require actions to be taken in Publish, Configure and Connect, however only a minority require to perform some actions during initialization.
The entry points will be executed in the order they have been added to the simulator.)")

      .def(
          "AddModel", &::Smp::ISimulator::AddModel, py::arg("model"),
          R"(This method adds a model to the models collection of the simulator, i.e. to the "Models" container.
This method raises an exception of type DuplicateName if the name of the new model conflicts with the name of an existing component (model or service).
The container for the models has no upper limit and thus the ContainerFull exception will never be thrown.
The method will never throw the InvalidObjectType exception either, as it gets a component implementing the IModel interface.)")

      .def(
          "AddService", &::Smp::ISimulator::AddService, py::arg("service"),
          R"(This method adds a user-defined service to the services collection,i.e. to the "Services" container.
This method raises an exception of type DuplicateName if the name of the new service conflicts with the name of an existing component(model or service).
The container for the services has no upper limit and thus the ContainerFull exception will never be thrown.
The method will never throw the InvalidObjectType exception either, as it gets a component implementing the IService interface.)")

      .def("GetLogger", &::Smp::ISimulator::GetLogger,
           py::return_value_policy::reference_internal,
           "Return interface to logger service.")

      .def("GetTimeKeeper", &::Smp::ISimulator::GetTimeKeeper,
           py::return_value_policy::reference_internal,
           "Return interface to time keeper service.")

      .def("GetScheduler", &::Smp::ISimulator::GetScheduler,
           py::return_value_policy::reference_internal,
           "Return interface to scheduler service.")

      .def("GetEventManager", &::Smp::ISimulator::GetEventManager,
           py::return_value_policy::reference_internal,
           "Return interface to event manager service.")

      .def("GetResolver", &::Smp::ISimulator::GetResolver,
           py::return_value_policy::reference_internal,
           "Return interface to resolver service.")

      .def("GetLinkRegistry", &::Smp::ISimulator::GetLinkRegistry,
           py::return_value_policy::reference_internal,
           "Return interface to link registry service.")

      // The created component is returned as a py::object rather than as a
      // Smp::IComponent*: the interfaces it really implements (IModel,
      // IComposite, IAggregate, ...) are only known at runtime, and reporting
      // the base interface would make every `AddModel(CreateInstance(...))` or
      // `parent=CreateInstance(...)` a static type error.
      .def(
          "CreateInstance",
          [](::Smp::ISimulator &self, ::Smp::Uuid uuid, ::Smp::String8 name,
             ::Smp::String8 description,
             ::Smp::IComposite *parent) -> py::object {
            return py::cast(
                self.CreateInstance(uuid, name, description, parent),
                py::return_value_policy::reference);
          },
          py::arg("uuid"), py::arg("name"), py::arg("description") = "",
          py::arg("parent") = nullptr,
          // keep_alive<0, 1> reproduces return_value_policy::reference_internal
          py::keep_alive<0, 1>(),
          "This method creates an instance of the component with the given "
          "unique identifier.")

      .def("GetFactory", &::Smp::ISimulator::GetFactory, py::arg("uuid"),
           py::return_value_policy::reference_internal,
           "This method returns the factory of the component with the given "
           "implementation identifier.")

      .def("GetFactories", &::Smp::ISimulator::GetFactories,
           py::return_value_policy::reference_internal,
           "This method returns all factories that have been registered with "
           "the simulator.")

      .def("GetTypeRegistry", &::Smp::ISimulator::GetTypeRegistry,
           py::return_value_policy::reference_internal,
           R"(Give access to the global type registry.
The type registry is typically a singleton, and must not be null, to allow use of existing types, and registration of new types.)")

      .def(
          "LoadLibrary",
          [](::Smp::ISimulator &self, ::Smp::String8 libraryName) {
            self.LoadLibrary(libraryName);
            // on success, load the library on Python side too to avoid dangling
            // references on typeinfo
            ::Xsmp::LoadLibrary(libraryName);
            // the library will stay open until the end of the process
          },
          py::arg("library_name"),
          R"(This operation loads a library of a package into memory.
At loading time, the Initialise() function of this library will be called.
At exiting or aborting time, the Finalise() function of this library will be called.)")

      .def("generate_python_type_hints", &generatePythonTypeHints,
           py::arg("path"),
           "Generate a python file describing the current simulator tree.")

      .doc() =
      R"(This interface gives access to the simulation environment state and state transitions. Further, it provides convenience methods to add models, and to add and retrieve simulation services.
This is a mandatory interface that every SMP compliant simulation environment has to implement.)";

  m.def("createSimulator", &createSimulator, py::arg("library_name"),
        py::arg("factory_name"), py::arg("name") = "Simulator",
        py::arg("description") = "", py::return_value_policy::take_ownership);
}

#endif // PYTHON_SMP_ISIMULATOR_H_
