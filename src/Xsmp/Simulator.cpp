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

#include <Smp/ComponentStateKind.h>
#include <Smp/IField.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Uuid.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/LibraryHelper.h>
#include <Xsmp/Simulator.h>
#include <Xsmp/StorageReader.h>
#include <Xsmp/StorageWriter.h>
#include <iterator>
#include <type_traits>

extern "C" ::Smp::ISimulator *createSimulator(::Smp::String8 name,
                                              ::Smp::String8 description) {
  return new ::Xsmp::Simulator(name, description);
}

namespace Xsmp {

Simulator::Simulator(::Smp::String8 name, ::Smp::String8 description)
    : _name(::Xsmp::Helper::checkName(name, nullptr)),
      _description(description ? description : ""),

      // initialize Services Container
      _services{SMP_SimulatorServices, "Services collection of the simulator",
                this, 0, -1},
      // initialize Models Container
      _models{SMP_SimulatorModels, "Models collection of the simulator", this,
              0, -1},
      // initialize factories
      _factories{this},
      // Hold Immediately Entry Point
      _holdImmediately{
          "HoldImmediately", "", this,
          [this] {
            this->_eventManager->Unsubscribe(
                ::Smp::Services::IEventManager::SMP_PreSimTimeChangeId,
                &_holdImmediately);
            this->Hold(true);
          }},
      // initialize the Type Registry
      _typeRegistry{this} {}

namespace {
template <typename Callable>
void recursive_action(::Smp::IComposite const *composite, Callable &&func);

/// Execute an action on a components and all its children
///
/// @param component the component
/// @param func the action to execute
template <typename Callable>
void recursive_action(::Smp::IComponent *component, Callable &&func) {
  func(component);
  if (auto *composite = dynamic_cast<::Smp::IComposite *>(component)) {
    recursive_action(composite, std::forward<Callable>(func));
  }
}

template <typename Callable>
void recursive_action(::Smp::IContainer const *container, Callable &&func) {
  if (const auto *components = container->GetComponents()) {
    for (auto *cmp : *components) {
      recursive_action(cmp, std::forward<Callable>(func));
    }
  }
}

/// Execute an action on all components inside a composite
/// @param composite the composite
/// @param func the action to execute
template <typename Callable>
void recursive_action(::Smp::IComposite const *composite, Callable &&func) {
  if (const auto *containers = composite->GetContainers()) {
    for (auto const *container : *containers) {
      recursive_action(container, std::forward<Callable>(func));
    }
  }
}

constexpr ::Smp::String8 initialiseSymbol = "Initialise";
constexpr ::Smp::String8 finaliseSymbol = "Finalise";
} // namespace
Simulator::~Simulator() {
  // Exit the simulation properly if the simulator is not already in exit or
  // abort state
  if (_state == ::Smp::SimulatorStateKind::SSK_Executing) {
    Hold(true);
  }
  if (_state == ::Smp::SimulatorStateKind::SSK_Standby) {
    Exit();
  }
  // finalise the libraries in reverse order
  while (!_libraries.empty()) {
    const auto &[name, handle] = _libraries.back();
    if (_logger) {
      _logger->Log(this, ("Unloading " + name + " library...").c_str(),
                   ::Smp::Services::ILogger::LMK_Debug);
    }

    // Call Finalise
    if ((*::Xsmp::GetSymbol<bool (*)(::Smp::ISimulator *simulator)>(
            handle, finaliseSymbol))(this)) {
      if (_logger) {
        _logger->Log(this,
                     ("Library " + name + " successfully unloaded.").c_str(),
                     ::Smp::Services::ILogger::LMK_Debug);
      }
    } else {
      if (_logger) {
        _logger->Log(this, ("Unable to unload " + name + " library.").c_str(),
                     ::Smp::Services::ILogger::LMK_Error);
      }
    }
    // remove the library handle
    _libraries.pop_back();
  }
}
::Smp::String8 Simulator::GetName() const { return _name.c_str(); }

::Smp::String8 Simulator::GetDescription() const {
  return _description.c_str();
}

::Smp::IObject *Simulator::GetParent() const { return nullptr; }

::Xsmp::Publication::Publication *
Simulator::CreatePublication(::Smp::IComponent *component) {
  return &_publications.emplace_back(component, &_typeRegistry);
}

void Simulator::Publish() {

  if (_state != ::Smp::SimulatorStateKind::SSK_Building) {
    if (_logger) {
      _logger->Log(this,
                   "Could not Publish the Simulation if simulator is not in "
                   "Building state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    }
    return;
  }

  recursive_action(this, [this](::Smp::IComponent *cmp) {
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Created)
      cmp->Publish(CreatePublication(cmp));
  });
}

void Simulator::Configure() {

  if (_state != ::Smp::SimulatorStateKind::SSK_Building) {
    if (_logger) {
      _logger->Log(this,
                   "Could not Configure the Simulation if simulator is not in "
                   "Building state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    }
    return;
  }

  recursive_action(this, [this](::Smp::IComponent *cmp) {
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Created)
      cmp->Publish(CreatePublication(cmp));
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Publishing)
      cmp->Configure(_logger, _linkRegistry);
  });
}

void Simulator::Connect() {

  if (_state != ::Smp::SimulatorStateKind::SSK_Building) {
    if (_logger)
      _logger->Log(this,
                   "Could not Connect the Simulation if simulator is not in "
                   "Building state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    return;
  }
  _state = ::Smp::SimulatorStateKind::SSK_Connecting;

  recursive_action(this, [this](::Smp::IComponent *cmp) {
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Created)
      cmp->Publish(CreatePublication(cmp));
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Publishing)
      cmp->Configure(_logger, _linkRegistry);
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Configured)
      cmp->Connect(this);
  });

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveConnectingId);

  _state = ::Smp::SimulatorStateKind::SSK_Initialising;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterInitialisingId);

  std::vector<::Smp::IEntryPoint *> entryPoints;
  entryPoints.swap(_initEntryPoints);
  for (auto const *entryPoint : entryPoints)
    entryPoint->Execute();

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveInitialisingId);

  _state = ::Smp::SimulatorStateKind::SSK_Standby;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStandbyId);
}

void Simulator::EmitGlobalEvent(::Smp::Services::EventId id) {
  _lastGlobalEventId = id;
  if (_eventManager)
    _eventManager->Emit(id);
  _lastGlobalEventId = -1;
}

void Simulator::Initialise() {

  if (_state != ::Smp::SimulatorStateKind::SSK_Standby ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveStandbyId) {
    if (_logger)
      _logger->Log(this,
                   "Could not Initialise the Simulation if simulator is not in "
                   "Standby state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    return;
  }
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStandbyId);

  _state = ::Smp::SimulatorStateKind::SSK_Initialising;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterInitialisingId);

  std::vector<::Smp::IEntryPoint *> entryPoints;
  entryPoints.swap(_initEntryPoints);
  for (auto const *entryPoint : entryPoints)
    entryPoint->Execute();

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveInitialisingId);

  _state = ::Smp::SimulatorStateKind::SSK_Standby;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStandbyId);
}
void Simulator::Run() {

  if (_state != ::Smp::SimulatorStateKind::SSK_Standby ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveStandbyId ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_EnterStandbyId) {
    if (_logger)
      _logger->Log(
          this,
          "Could not Run the Simulation if simulator is not in Standby state.",
          ::Smp::Services::ILogger::LMK_Warning);
    return;
  }
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStandbyId);

  _state = ::Smp::SimulatorStateKind::SSK_Executing;

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterExecutingId);
}

void Simulator::Run(::Smp::Duration duration) {

  if (_state != ::Smp::SimulatorStateKind::SSK_Standby ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveStandbyId ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_EnterStandbyId) {
    if (_logger)
      _logger->Log(
          this,
          "Could not Run the Simulation if simulator is not in Standby state.",
          ::Smp::Services::ILogger::LMK_Warning);
    return;
  }

  ::Xsmp::EntryPoint hold{"hold",
                          "call simulator hold after the specified duration",
                          this, [this] { this->Hold(false); }};
  if (_scheduler)
    _scheduler->AddSimulationTimeEvent(&hold, duration);
  Run();
}

void Simulator::Hold(::Smp::Bool immediate) {

  if (_state != ::Smp::SimulatorStateKind::SSK_Executing ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveExecutingId) {
    if (_logger)
      _logger->Log(this,
                   "Could not Hold the Simulation if simulator is not in "
                   "Executing state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    return;
  }

  if (immediate) {

    EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveExecutingId);

    _state = ::Smp::SimulatorStateKind::SSK_Standby;

    EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStandbyId);
  } else {
    if (_eventManager)
      _eventManager->Subscribe(
          ::Smp::Services::IEventManager::SMP_PreSimTimeChangeId,
          &_holdImmediately);
  }
}

enum class PersistKind { PERSIST, COMPONENT, COMPOSITE, CONTAINER, FIELD };
namespace {
void Store(::Smp::IObject *obj, ::Smp::IStorageWriter *writer) {

  PersistKind kind;
  if (auto *persist = dynamic_cast<::Smp::IPersist *>(obj)) {
    kind = PersistKind::PERSIST;
    writer->Store(&kind, sizeof(PersistKind));
    persist->Store(writer);
  }
  if (auto const *component = dynamic_cast<::Smp::IComponent *>(obj)) {
    kind = PersistKind::COMPONENT;
    writer->Store(&kind, sizeof(PersistKind));
    kind = PersistKind::FIELD;
    if (auto *fields = component->GetFields())
      for (auto *field : *fields) {
        writer->Store(&kind, sizeof(PersistKind));
        field->Store(writer);
      }
  }
  if (auto const *composite = dynamic_cast<::Smp::IComposite *>(obj)) {
    kind = PersistKind::COMPOSITE;
    writer->Store(&kind, sizeof(PersistKind));
    kind = PersistKind::CONTAINER;
    if (composite->GetContainers())
      for (auto *container : *composite->GetContainers()) {
        writer->Store(&kind, sizeof(PersistKind));
        Store(container, writer);
      }
  }
}
} // namespace
static constexpr ::Smp::String8 PERSIST_FILENAME = "simulator.bin";
void Simulator::Store(::Smp::String8 filename) {

  if (_state != ::Smp::SimulatorStateKind::SSK_Standby ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveStandbyId) {
    if (_logger)
      _logger->Log(this,
                   "Could not Store the Simulation if simulator is not in "
                   "Standby state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    return;
  }
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStandbyId);

  _state = ::Smp::SimulatorStateKind::SSK_Storing;

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStoringId);

  StorageWriter writer{filename, PERSIST_FILENAME, this};

  ::Xsmp::Store(this, &writer);

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStoringId);
  _state = ::Smp::SimulatorStateKind::SSK_Standby;

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStandbyId);
}
namespace {
void check(const ::Smp::IObject *obj, ::Smp::IStorageReader *reader,
           PersistKind expectedKind) {
  PersistKind kind;
  reader->Restore(&kind, sizeof(PersistKind));

  if (kind != expectedKind)
    ::Xsmp::Exception::throwCannotRestore(obj, "Wrong check");
}
void Restore(::Smp::IObject *obj, ::Smp::IStorageReader *reader) {
  if (auto *persist = dynamic_cast<::Smp::IPersist *>(obj)) {
    check(obj, reader, PersistKind::PERSIST);
    persist->Restore(reader);
  }
  if (auto const *component = dynamic_cast<::Smp::IComponent *>(obj)) {
    check(obj, reader, PersistKind::COMPONENT);
    if (auto *fields = component->GetFields())
      for (auto *field : *fields) {
        check(obj, reader, PersistKind::FIELD);
        field->Restore(reader);
      }
  }

  if (auto const *composite = dynamic_cast<::Smp::IComposite *>(obj)) {
    check(obj, reader, PersistKind::COMPOSITE);
    if (composite->GetContainers())
      for (auto *container : *composite->GetContainers()) {
        check(obj, reader, PersistKind::CONTAINER);
        Restore(container, reader);
      }
  }
}
} // namespace
void Simulator::Restore(::Smp::String8 filename) {

  if (_state != ::Smp::SimulatorStateKind::SSK_Standby ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveStandbyId) {
    if (_logger)
      _logger->Log(this,
                   "Could not Restore the Simulation if simulator is not in "
                   "Standby state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    return;
  }
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStandbyId);

  _state = ::Smp::SimulatorStateKind::SSK_Restoring;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterRestoringId);
  StorageReader reader{filename, PERSIST_FILENAME, this};

  ::Xsmp::Restore(this, &reader);

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveRestoringId);
  _state = ::Smp::SimulatorStateKind::SSK_Standby;

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStandbyId);
}

void Simulator::Reconnect(::Smp::IComponent *root) {

  if (_state != ::Smp::SimulatorStateKind::SSK_Standby ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveStandbyId) {
    if (_logger)
      _logger->Log(this,
                   "Could not Reconnect the Simulation if simulator is not in "
                   "Standby state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    return;
  }
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStandbyId);
  _state = ::Smp::SimulatorStateKind::SSK_Reconnecting;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterReconnectingId);

  if (auto const *composite = dynamic_cast<::Smp::IComposite *>(root))
    recursive_action(composite, [this](::Smp::IComponent *cmp) {
      if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Created)
        cmp->Publish(CreatePublication(cmp));
      if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Publishing)
        cmp->Configure(_logger, _linkRegistry);
      if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Configured)
        cmp->Connect(this);
    });

  _state = ::Smp::SimulatorStateKind::SSK_Standby;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStandbyId);
}

void Simulator::Exit() {

  if (_state != ::Smp::SimulatorStateKind::SSK_Standby ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveStandbyId) {
    if (_logger)
      _logger->Log(this,
                   "Could not Exit from Simulation while simulator is not in "
                   "Standby state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    return;
  }

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStandbyId);
  _state = ::Smp::SimulatorStateKind::SSK_Exiting;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterExitingId);

  // Disconnect all components properly
  recursive_action(this, [](::Smp::IComponent *cmp) {
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Connected)
      cmp->Disconnect();
  });
}

void Simulator::Abort() {

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterAbortingId);
  _state = ::Smp::SimulatorStateKind::SSK_Aborting;
}

::Smp::SimulatorStateKind Simulator::GetState() const { return _state; }

void Simulator::AddInitEntryPoint(::Smp::IEntryPoint *entryPoint) {

  switch (_state) {
  case ::Smp::SimulatorStateKind::SSK_Building:
  case ::Smp::SimulatorStateKind::SSK_Connecting:
  case ::Smp::SimulatorStateKind::SSK_Standby:
    _initEntryPoints.push_back(entryPoint);
    break;
  default:
    if (_logger)
      _logger->Log(entryPoint,
                   "Unable to Add an Init EntryPoint in wrong Simulator State.",
                   ::Smp::Services::ILogger::LMK_Warning);
    break;
  }
}

void Simulator::AddModel(::Smp::IModel *model) {

  switch (_state) {
  case ::Smp::SimulatorStateKind::SSK_Building:
  case ::Smp::SimulatorStateKind::SSK_Connecting:
  case ::Smp::SimulatorStateKind::SSK_Standby:
    _models.AddComponent(model);
    break;
  default:
    ::Xsmp::Exception::throwInvalidSimulatorState(this, _state);
  }
}

void Simulator::AddService(::Smp::IService *service) {
  switch (_state) {
  case ::Smp::SimulatorStateKind::SSK_Building:
  case ::Smp::SimulatorStateKind::SSK_Connecting:
  case ::Smp::SimulatorStateKind::SSK_Standby:
    _services.AddComponent(service);
    break;
  default:
    ::Xsmp::Exception::throwInvalidSimulatorState(this, _state);
  }
  /// Helper that register a standard service in the simulator
  auto RegisterService = [service, this](auto *&value) {
    if (value)
      return;
    if ((value =
             dynamic_cast<std::remove_reference_t<decltype(value)>>(service)) &&
        _logger)
      _logger->Log(
          this,
          ("Service '" + ::Xsmp::Helper::TypeName(value) + "' registered.")
              .c_str(),
          ::Smp::Services::ILogger::LMK_Information);
  };
  RegisterService(_logger);
  RegisterService(_eventManager);
  RegisterService(_linkRegistry);
  RegisterService(_resolver);
  RegisterService(_scheduler);
  RegisterService(_timeKeeper);
}

::Smp::IService *Simulator::GetService(::Smp::String8 name) const {
  auto it = _services.find(name);
  return it != _services.end() ? *it : nullptr;
}

::Smp::Services::ILogger *Simulator::GetLogger() const { return _logger; }

::Smp::Services::ITimeKeeper *Simulator::GetTimeKeeper() const {
  return _timeKeeper;
}

::Smp::Services::IScheduler *Simulator::GetScheduler() const {
  return _scheduler;
}

::Smp::Services::IEventManager *Simulator::GetEventManager() const {
  return _eventManager;
}

::Smp::Services::IResolver *Simulator::GetResolver() const { return _resolver; }

::Smp::Services::ILinkRegistry *Simulator::GetLinkRegistry() const {
  return _linkRegistry;
}

void Simulator::RegisterFactory(::Smp::IFactory *componentFactory) {
  _factories.Add(componentFactory);
}

::Smp::IComponent *Simulator::CreateInstance(::Smp::Uuid uuid,
                                             ::Smp::String8 name,
                                             ::Smp::String8 description,
                                             ::Smp::IComposite *parent) {
  auto *factory = GetFactory(uuid);

  return factory ? factory->CreateInstance(name, description, parent) : nullptr;
}

::Smp::IFactory *Simulator::GetFactory(::Smp::Uuid uuid) const {
  return _factories.at(uuid);
}

const ::Smp::FactoryCollection *Simulator::GetFactories() const {
  return &_factories;
}

::Smp::Publication::ITypeRegistry *Simulator::GetTypeRegistry() const {
  return &_typeRegistry;
}

void Simulator::LoadLibrary(::Smp::String8 libraryPath) {

  if (_logger)
    _logger->Log(
        this,
        ("Loading '" + std::string(libraryPath) + "' library ...").c_str(),
        ::Smp::Services::ILogger::LMK_Debug);

  void *handle = ::Xsmp::LoadLibrary(libraryPath);

  if (!handle) {
    auto error = ::Xsmp::GetLastError();
    if (_logger)
      _logger->Log(this, error.c_str(), ::Smp::Services::ILogger::LMK_Error);
    ::Xsmp::Exception::throwLibraryNotFound(this, libraryPath, error);
  }

  auto initialise = ::Xsmp::GetSymbol<bool (*)(
      ::Smp::ISimulator *simulator,
      ::Smp::Publication::ITypeRegistry *type_registry)>(handle,
                                                         initialiseSymbol);

  if (!initialise) {
    std::string msg =
        std::string("Library '") + libraryPath +
        "' does not provide function 'bool Initialize(::Smp::ISimulator *, "
        "::Smp::Publication::ITypeRegistry *)': " +
        ::Xsmp::GetLastError();
    if (_logger)
      _logger->Log(this, msg.c_str(), ::Smp::Services::ILogger::LMK_Error);
    ::Xsmp::Exception::throwInvalidLibrary(this, libraryPath, msg);
  }

  // check that Finalise exist
  if (!::Xsmp::GetSymbol<bool (*)(::Smp::ISimulator *simulator)>(
          handle, finaliseSymbol)) {
    std::string msg = std::string("Library '") + libraryPath +
                      "' does not provide function 'bool Finalise()': " +
                      ::Xsmp::GetLastError();
    if (_logger)
      _logger->Log(this, msg.c_str(), ::Smp::Services::ILogger::LMK_Error);
    ::Xsmp::Exception::throwInvalidLibrary(this, libraryPath, msg);
  }

  if ((*initialise)(this, &_typeRegistry)) {
    if (_logger)
      _logger->Log(
          this,
          ("Library '" + std::string(libraryPath) + "' successfully loaded.")
              .c_str(),
          ::Smp::Services::ILogger::LMK_Debug);
  } else {
    std::string msg =
        std::string("Initialise() of library '") + libraryPath + "' failed.";
    if (_logger)
      _logger->Log(this, msg.c_str(), ::Smp::Services::ILogger::LMK_Error);
    ::Xsmp::Exception::throwInvalidLibrary(this, libraryPath, msg);
  }
  _libraries.emplace_back(libraryPath, handle);
}

} // namespace Xsmp
