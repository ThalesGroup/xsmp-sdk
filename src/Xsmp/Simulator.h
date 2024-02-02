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

#ifndef XSMP_SIMULATOR_H_
#define XSMP_SIMULATOR_H_

#include <Smp/IFactory.h>
#include <Smp/IModel.h>
#include <Smp/IService.h>
#include <Smp/ISimulator.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/EventId.h>
#include <Smp/SimulatorStateKind.h>
#include <Xsmp/Composite.h>
#include <Xsmp/Container.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/FactoryCollection.h>
#include <Xsmp/Publication/Publication.h>
#include <Xsmp/Publication/TypeRegistry.h>
#include <list>
#include <string>
#include <utility>
#include <vector>

namespace Smp::Services {
class IEventManager;
class IScheduler;
class ILogger;
class ITimeKeeper;
} // namespace Smp::Services

extern "C" ::Smp::ISimulator* createSimulator(::Smp::String8 name,
        ::Smp::String8 description);

namespace Xsmp {

class Simulator final: public ::Xsmp::Composite, public ::Smp::ISimulator {
public:
    Simulator(::Smp::String8 name = "XsmpSimulator",
            ::Smp::String8 description = "Simulator implementation from XSMP.");
    ~Simulator() override;
    /// Simulator cannot be copied
    Simulator(const Simulator&) = delete;
    /// Simulator cannot be copied
    Simulator& operator=(const Simulator&) = delete;

    ::Smp::String8 GetName() const override;
    ::Smp::String8 GetDescription() const override;
    ::Smp::IObject* GetParent() const override;

    /// This method asks the simulation environment to call all
    /// initialisation entry points again.
    /// This method can only be called when in Standby state, and enters
    /// Initialising state. After completion, the simulator automatically
    /// returns to Standby state.
    /// The entry points will be executed in the order they have been added
    /// to the simulator using the AddInitEntryPoint() method.
    void Initialise() override;

    /// This method asks the simulation environment to call the Publish()
    /// method of all service and model instances in the component
    /// hierarchy which are still in Created state.
    ///
    /// This method must only be called when in Building state.
    /// @remarks This method is typically called by an external component
    ///          after creating new model instances, typically from
    ///          information in an SMDL Assembly.
    ///          The Publish() operation will traverse recursively through
    ///          both the "Models" container and then the "Services"
    ///          container of the simulator. It will call the Publish()
    ///          operation of each component in CSK_Created state, and then
    ///          immediately on all its child components.
    void Publish() override;

    /// This method asks the simulation environment to call the Configure()
    /// method of all service and model instances which are still in
    /// Publishing state.
    ///
    /// This method must only be called when in Building state.
    /// @remarks This method is typically called by an external component
    ///          after setting field values of new model instances,
    ///          typically using the information in an SMDL Assembly or
    ///          SMDL Configuration.
    ///          The Configure() operation will traverse recursively
    ///          through both the "Models" container and then the
    ///          "Services" container of the simulator. For each component,
    ///          it will first call the Publish() operation (if the
    ///          component is still in CSK_Created state), then the
    ///          Configure() operation (if the component is in
    ///          CSK_Publishing state), and then the simulator will
    ///          immediately perform the same operation(s) on all child
    ///          components of the component.
    void Configure() override;

    /// This method informs the simulation environment that the hierarchy
    /// of model instances has been configured, and can now be connected to
    /// the simulator. Thus, the simulation environment calls the Connect()
    /// method of all service and model instances.
    /// In this method, the simulation environment first enters Connecting
    /// state and calls the Connect() method of every model in the model
    /// hierarchy, then enters Initialising state and calls the
    /// initialisation entry points, and finally enters Standby state.
    /// This method must only be called when in Building state.
    /// @remarks This method is typically called by an external component
    ///          after configuring all model instances.
    ///          The Connect() operation will traverse recursively through
    ///          both the "Models" container and then the "Services"
    ///          container of the simulator. For each component, it will
    ///          first call the Publish() and Configure() operations (if
    ///          the component is still in CSK_Created state), then the
    ///          Connect() operation (if the component is in CSK_Configured
    ///          state), and then the simulator will immediately perform
    ///          the same operation(s) on all child components of the
    ///          component.
    void Connect() override;

    /// This method changes from Standby to Executing state.
    /// This method must only be called when in Standby state, and enters
    /// Executing state.
    void Run() override;

    /// This method changes from Executing to Standby state.
    /// This method must only be called when in Executing state, and enters
    /// Standby state.
    /// @param   immediate True if the simulator shall stop immediately
    ///          after completion of the current scheduler event, false if
    ///          the simulator shall still process all events of the
    ///          current simulation time before entering Standby state.
    void Hold(::Smp::Bool immediate) override;

    /// This method is used to store a state vector to file.
    /// This method must only be called when in Standby state, and enters
    /// Storing state. On completion, it automatically returns to Standby
    /// state.
    /// @param   filename Name including the full path to use for
    ///          simulation state vector file.
    void Store(::Smp::String8 filename) override;

    /// This method is used to restore a state vector from file.
    /// This method must only be called when in Standby state, and enters
    /// Restoring state. On completion, it automatically returns to Standby
    /// state.
    /// @param   filename Name including the full path of simulation state
    ///          vector file.
    void Restore(::Smp::String8 filename) override;

    /// This method asks the simulation environment to reconnect the
    /// component hierarchy starting at the given root component.
    /// This method must only be called when in Standby state.
    /// @remarks This method is typically called after creating additional
    ///          model instances and adding them to the existing model
    ///          hierarchy.
    ///          The simulation environment has to ensure that all models
    ///          under the given root (but not the root itself) are
    ///          published, configured and connected, so that all child
    ///          models are finally in Connected state.
    /// @param   root Root component to start reconnecting from. This can
    ///          be the parent component of a newly added model, or e.g.
    ///          the simulator itself.
    void Reconnect(::Smp::IComponent *root) override;

    /// This method is used for a normal termination of a simulation.
    /// This method must only be called when in Standby state, and enters
    /// Exiting state.
    void Exit() override;

    /// This method is used for an abnormal termination of a simulation.
    /// This method can be called from any other state, and enters Aborting
    /// state.
    void Abort() override;

    /// Return the current simulator state.
    /// @return  Current simulator state.
    ::Smp::SimulatorStateKind GetState() const override;

    /// This method can be used to add entry points that shall be executed
    /// in the Initialising state.
    /// If the simulation is in Building, Connecting or Standby state, it
    /// adds the entry point to the list of entry points to be executed
    /// once the simulation reaches Initialising state.
    /// If the simulation is not in Building, Connecting or Standby state,
    /// then it returns and no action is taken.
    /// This allows components to subscribe to a callback during
    /// initialization phase since there are only explicit methods defined
    /// for Publish, Configure and Connect. This choice is taken since most
    /// models require actions to be taken in Publish, Configure and
    /// Connect, however only a minority require to perform some actions
    /// during initialization.
    /// The entry points will be executed in the order they have been added
    /// to the simulator.
    /// @param   entryPoint Entry point to execute in Initialising state.
    void AddInitEntryPoint(::Smp::IEntryPoint *entryPoint) override;

    /// This method adds a model to the models collection of the simulator,
    /// i.e. to the "Models" container.
    /// This method raises an exception of type DuplicateName if the name
    /// of the new model conflicts with the name of an existing component
    /// (model or service).
    /// The container for the models has no upper limit and thus the
    /// ContainerFull exception will never be thrown.
    /// The method will never throw the InvalidObjectType exception either,
    /// as it gets a component implementing the IModel interface.
    /// @param   model New model to add to collection of root models, i.e.
    ///          to the "Models" container.
    /// @throws  ::Smp::DuplicateName
    /// @throws  ::Smp::InvalidSimulatorState
    void AddModel(::Smp::IModel *model) override;

    /// This method adds a user-defined service to the services collection,
    /// i.e. to the "Services" container.
    /// This method raises an exception of type DuplicateName if the name
    /// of the new service conflicts with the name of an existing component
    /// (model or service).
    /// The container for the services has no upper limit and thus the
    /// ContainerFull exception will never be thrown.
    /// The method will never throw the InvalidObjectType exception either,
    /// as it gets a component implementing the IService interface.
    /// @remarks It is recommended that custom services include a project
    ///          or company acronym as prefix in their name, to avoid
    ///          collision of service names.
    ///
    ///          The container for the services has no upper limit and thus
    ///          the ContainerFull exception will never be thrown.
    ///          The method will never throw the InvalidObjectType
    ///          exception, as it expects a component implementing the
    ///          IService interface.
    /// @param   service Service to add to services collection.
    /// @throws  ::Smp::DuplicateName
    /// @throws  ::Smp::InvalidSimulatorState
    void AddService(::Smp::IService *service) override;

    /// Query for a service component by its name.
    /// The returned component is null if no service with the given name
    /// could be found. Standard names are defined for the standardised
    /// services, while custom services use custom names.
    /// The existence of custom services is not guaranteed, so models
    /// should expect to get a null reference.
    /// @param   name Service name.
    /// @return  Service with the given name, or null if no service with
    ///          the given name could be found.
    ::Smp::IService* GetService(::Smp::String8 name) const override;

    /// Return interface to logger service.
    /// @remarks This is a type-safe convenience method, to avoid having to
    ///          use the generic GetService() method. For the standardised
    ///          services, it is recommended to use the convenience
    ///          methods, which are guaranteed to return a valid reference.
    /// @return  Interface to mandatory logger service.
    ::Smp::Services::ILogger* GetLogger() const override;

    /// Return interface to time keeper service.
    /// @remarks This is a type-safe convenience method, to avoid having to
    ///          use the generic GetService() method. For the standardised
    ///          services, it is recommended to use the convenience
    ///          methods, which are guaranteed to return a valid reference.
    /// @return  Interface to mandatory time keeper service.
    ::Smp::Services::ITimeKeeper* GetTimeKeeper() const override;

    /// Return interface to scheduler service.
    /// @remarks This is a type-safe convenience method, to avoid having to
    ///          use the generic GetService() method. For the standardised
    ///          services, it is recommended to use the convenience
    ///          methods, which are guaranteed to return a valid reference.
    /// @return  Interface to mandatory scheduler service.
    ::Smp::Services::IScheduler* GetScheduler() const override;

    /// Return interface to event manager service.
    /// @remarks This is a type-safe convenience method, to avoid having to
    ///          use the generic GetService() method. For the standardised
    ///          services, it is recommended to use the convenience
    ///          methods, which are guaranteed to return a valid reference.
    /// @return  Interface to mandatory event manager service.
    ::Smp::Services::IEventManager* GetEventManager() const override;

    /// Return interface to resolver service.
    /// @remarks This is a type-safe convenience method, to avoid having to
    ///          use the generic GetService() method. For the standardised
    ///          services, it is recommended to use the convenience
    ///          methods, which are guaranteed to return a valid reference.
    /// @return  Interface to mandatory resolver service.
    ::Smp::Services::IResolver* GetResolver() const override;

    /// Return interface to link registry service.
    /// @remarks This is a type-safe convenience method, to avoid having to
    ///          use the generic GetService() method. For the standardised
    ///          services, it is recommended to use the convenience
    ///          methods, which are guaranteed to return a valid reference.
    /// @return  Interface to mandatory link registry service.
    ::Smp::Services::ILinkRegistry* GetLinkRegistry() const override;

    /// This method registers a component factory with the simulator. The
    /// simulator can use this factory to create component instances of the
    /// component implementation in its CreateInstance() method.
    /// The memory management of the factory is passed to the simulator,
    /// who is in charge of deleting the factory at the exiting or aborting
    /// state.
    /// This method raises an exception of type DuplicateUuid if another
    /// factory has been registered using the same uuid already.
    /// @remarks This method is typically called early in the Building
    ///          state to register the available component factories before
    ///          the hierarchy of model instances is created.
    /// @param   componentFactory Factory to create instance of the
    ///          component implementation.
    /// @throws  ::Smp::DuplicateUuid
    void RegisterFactory(::Smp::IFactory *componentFactory) override;

    /// This method creates an instance of the component with the given
    /// unique identifier.
    /// @remarks This method is typically called during Creating state when
    ///          building the hierarchy of models.
    /// @param   uuid Unique identifier of the component to create.
    /// @param   name Name of the new instance.
    /// f the name provided is not a valid object name, an exception of
    /// type InvalidObjectName is raised.
    /// @param   description Description of the new instance.
    /// @param   parent Parent object of the new instance.
    /// @return  New instance of the component with the given
    ///          implementation identifier or null in case no factory for
    ///          the given implementation identifier has been registered.
    /// @throws  ::Smp::InvalidObjectName
    ::Smp::IComponent* CreateInstance(::Smp::Uuid uuid, ::Smp::String8 name,
            ::Smp::String8 description, ::Smp::IComposite *parent) override;

    /// This method returns the factory of the component with the given
    /// implementation identifier.
    /// @param   uuid Unique identifier of the component to get the factory
    ///          for.
    /// @return  Factory of the component with the given implementation
    ///          identifier or null in case no factory for the given
    ///          implementation identifier has been registered.
    ::Smp::IFactory* GetFactory(::Smp::Uuid uuid) const override;

    /// This method returns all factories that have been registered with
    /// the simulator.
    /// @return  Collection of factories.
    const ::Smp::FactoryCollection* GetFactories() const override;

    /// Give access to the global type registry.
    /// The type registry is typically a singleton, and must not be null,
    /// to allow use of existing types, and registration of new types.
    /// @return  Reference to global type registry.
    ::Smp::Publication::ITypeRegistry* GetTypeRegistry() const override;

    /// This operation loads a library of a package into memory.
    /// At loading time, the Initialise() function of this library will be
    /// called.
    /// At exiting or aborting time, the Finalise() function of this
    /// library will be called.
    /// @param   libraryPath Path to the library to load.
    /// This needs to be a valid path name given the constraints of the
    /// operating system.
    /// @throws  ::Smp::InvalidLibrary
    /// @throws  ::Smp::LibraryNotFound
    void LoadLibrary(::Smp::String8 libraryPath) override;

    /// For tests: run for a specific duration
    /// @param duration the execution Duration
    void Run(::Smp::Duration duration);

private:

    void EmitGlobalEvent(::Smp::Services::EventId id);

    [[nodiscard]] ::Xsmp::Publication::Publication* CreatePublication(
            ::Smp::IComponent *component);

    std::string _name;
    std::string _description;

    std::vector<std::pair<std::string, void*>> _libraries { };

    ::Smp::Services::EventId _lastGlobalEventId = -1;

    Container<::Smp::IService> _services;
    Container<::Smp::IModel> _models;

    std::vector<::Smp::IEntryPoint*> _initEntryPoints { };

    std::list<::Xsmp::Publication::Publication> _publications { };

    FactoryCollection _factories;

    ::Xsmp::EntryPoint _holdImmediately;

    // standard services
    ::Smp::Services::ILinkRegistry *_linkRegistry = nullptr;
    ::Smp::Services::IResolver *_resolver = nullptr;
    ::Smp::Services::ITimeKeeper *_timeKeeper = nullptr;
    ::Smp::Services::IEventManager *_eventManager = nullptr;
    ::Smp::Services::IScheduler *_scheduler = nullptr;
    ::Smp::Services::ILogger *_logger = nullptr;

    mutable ::Xsmp::Publication::TypeRegistry _typeRegistry;
    ::Smp::SimulatorStateKind _state = ::Smp::SimulatorStateKind::SSK_Building;
};

} // namespace Xsmp

#endif // XSMP_SIMULATOR_H_
