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
#include <Smp/IComposite.h>
#include <Smp/IFactory.h>
#include <Smp/IField.h>
#include <Smp/Publication/IType.h>
#include <Smp/IModel.h>
#include <Smp/IPersist.h>
#include <Smp/IService.h>
#include <Smp/IStorageReader.h>
#include <Smp/IStorageWriter.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/ITypeRegistry.h>
#include <Smp/Services/EventId.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ILinkRegistry.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/IResolver.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/SimulatorStateKind.h>
#include <Smp/Uuid.h>
#include <Xsmp/EntryPoint.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/LibraryHelper.h>
#include <Xsmp/Publication/Publication.h>
#include <Xsmp/Simulator.h>
#include <Xsmp/StorageReader.h>
#include <Xsmp/StorageWriter.h>
#include <cstdint>
#include <exception>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

extern "C" ::Smp::ISimulator *createSimulator(::Smp::String8 name,
                                              ::Smp::String8 description) {
  return new ::Xsmp::Simulator(name, description);
}

namespace Xsmp {

Simulator::Simulator(::Smp::String8 name, ::Smp::String8 description)
    : _name(::Xsmp::Helper::checkName(name, nullptr)),
      _description(description),

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
  // A destructor is implicitly noexcept: an exception thrown by a model in its
  // exit sequence must not terminate the process.
  try {
    // Exit the simulation properly if the simulator is not already in exit or
    // abort state
    if (_state == ::Smp::SimulatorStateKind::SSK_Executing) {
      Hold(true);
    }
    if (_state == ::Smp::SimulatorStateKind::SSK_Standby) {
      Exit();
    }
  } catch (const std::exception &e) {
    if (_logger) {
      _logger->Log(this,
                   (std::string("Exception thrown while exiting the "
                                "simulation: ") +
                    e.what())
                       .c_str(),
                   ::Smp::Services::ILogger::LMK_Error);
    }
  } catch (...) {
    if (_logger) {
      _logger->Log(this, "Exception thrown while exiting the simulation.",
                   ::Smp::Services::ILogger::LMK_Error);
    }
  }

  // finalise the libraries in reverse order
  while (!_libraries.empty()) {
    const auto &[name, handle] = _libraries.back();
    if (_logger) {
      _logger->Log(this, ("Finalising " + name + " library...").c_str(),
                   ::Smp::Services::ILogger::LMK_Debug);
    }

    // Call Finalise. Its existence is checked by LoadLibrary.
    try {
      if ((*::Xsmp::GetSymbol<bool (*)(::Smp::ISimulator *simulator)>(
              handle, finaliseSymbol))(this)) {
        if (_logger) {
          _logger->Log(this,
                       ("Library " + name + " successfully finalised.").c_str(),
                       ::Smp::Services::ILogger::LMK_Debug);
        }
      } else if (_logger) {
        _logger->Log(this, ("Unable to finalise " + name + " library.").c_str(),
                     ::Smp::Services::ILogger::LMK_Error);
      }
    } catch (...) {
      if (_logger) {
        _logger->Log(
            this,
            ("Exception thrown by Finalise() of library " + name + ".").c_str(),
            ::Smp::Services::ILogger::LMK_Error);
      }
    }
    // The library stays mapped: the models it created are held by _models and
    // _publications, which are destroyed after this body runs, and their
    // destructors and vtables live in the library.
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
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Created) {
      cmp->Publish(CreatePublication(cmp));
    }
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
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Created) {
      cmp->Publish(CreatePublication(cmp));
    }
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Publishing) {
      cmp->Configure(_logger, _linkRegistry);
    }
  });
}

void Simulator::Connect() {

  if (_state != ::Smp::SimulatorStateKind::SSK_Building) {
    if (_logger) {
      _logger->Log(this,
                   "Could not Connect the Simulation if simulator is not in "
                   "Building state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    }
    return;
  }
  _state = ::Smp::SimulatorStateKind::SSK_Connecting;

  recursive_action(this, [this](::Smp::IComponent *cmp) {
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Created) {
      cmp->Publish(CreatePublication(cmp));
    }
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Publishing) {
      cmp->Configure(_logger, _linkRegistry);
    }
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Configured) {
      cmp->Connect(this);
    }
  });

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveConnectingId);

  _state = ::Smp::SimulatorStateKind::SSK_Initialising;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterInitialisingId);

  std::vector<::Smp::IEntryPoint *> entryPoints;
  entryPoints.swap(_initEntryPoints);
  for (auto const *entryPoint : entryPoints) {
    entryPoint->Execute();
  }

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveInitialisingId);

  _state = ::Smp::SimulatorStateKind::SSK_Standby;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStandbyId);
}

void Simulator::EmitGlobalEvent(::Smp::Services::EventId eventId) {
  _lastGlobalEventId = eventId;
  if (_eventManager) {
    _eventManager->Emit(eventId);
  }
  _lastGlobalEventId = -1;
}

void Simulator::Initialise() {

  if (_state != ::Smp::SimulatorStateKind::SSK_Standby ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveStandbyId) {
    if (_logger) {
      _logger->Log(this,
                   "Could not Initialise the Simulation if simulator is not in "
                   "Standby state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    }
    return;
  }
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStandbyId);

  _state = ::Smp::SimulatorStateKind::SSK_Initialising;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterInitialisingId);

  std::vector<::Smp::IEntryPoint *> entryPoints;
  entryPoints.swap(_initEntryPoints);
  for (auto const *entryPoint : entryPoints) {
    entryPoint->Execute();
  }
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
    if (_logger) {
      _logger->Log(
          this,
          "Could not Run the Simulation if simulator is not in Standby state.",
          ::Smp::Services::ILogger::LMK_Warning);
    }
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
    if (_logger) {
      _logger->Log(
          this,
          "Could not Run the Simulation if simulator is not in Standby state.",
          ::Smp::Services::ILogger::LMK_Warning);
    }
    return;
  }

  ::Smp::Services::EventId holdId = -1;
  ::Xsmp::EntryPoint hold{"hold",
                          "call simulator hold after the specified duration",
                          this, [this, &holdId] {
                            holdId = -1;
                            this->Hold(false);
                          }};
  if (_scheduler) {
    holdId = _scheduler->AddSimulationTimeEvent(&hold, duration);
  }

  Run();

  // `hold` lives on the stack: an event that has not been executed must be
  // removed, otherwise a later Run() executes a destroyed entry point
  if (_scheduler && holdId != -1) {
    _scheduler->RemoveEvent(holdId);
  }
}

void Simulator::Hold(::Smp::Bool immediate) {

  if (_state != ::Smp::SimulatorStateKind::SSK_Executing ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveExecutingId) {
    if (_logger) {
      _logger->Log(this,
                   "Could not Hold the Simulation if simulator is not in "
                   "Executing state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    }
    return;
  }

  if (immediate) {

    EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveExecutingId);

    _state = ::Smp::SimulatorStateKind::SSK_Standby;

    EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStandbyId);
  } else {
    if (_eventManager) {
      _eventManager->Subscribe(
          ::Smp::Services::IEventManager::SMP_PreSimTimeChangeId,
          &_holdImmediately);
    }
  }
}

namespace {
enum class PersistKind : std::uint8_t { PERSIST, FIELD };

/// The state vector is a positional format: each entry is tagged with the
/// identity of the object it belongs to, so that restoring a state stored by a
/// different model tree is refused instead of reading one object's bytes into
/// another. The hash is computed here rather than taken from std::hash, whose
/// result is not stable across implementations.
::Smp::UInt64 hash(std::string_view value) {
  ::Smp::UInt64 result = 14695981039346656037ULL;
  for (const auto character : value) {
    result ^= static_cast<unsigned char>(character);
    result *= 1099511628211ULL;
  }
  return result;
}

::Smp::UInt64 identity(const ::Smp::IObject *object) {
  return hash(::Xsmp::Helper::GetPath(object));
}

/// A field is identified by its path and its type, so that a field whose type
/// changed is not restored from the bytes of the previous one.
::Smp::UInt64 identity(const ::Smp::IField *field) {
  const auto *type = field->GetType();
  return hash(::Xsmp::Helper::GetPath(field) + '|' +
              (type ? type->GetName() : ""));
}

void storeEntry(::Smp::IStorageWriter *writer, PersistKind kind,
                ::Smp::UInt64 objectId) {
  writer->Store(&kind, sizeof(kind));
  writer->Store(&objectId, sizeof(objectId));
}

void check(const ::Smp::IObject *obj, ::Smp::IStorageReader *reader,
           PersistKind expectedKind, ::Smp::UInt64 expectedId) {
  PersistKind kind{};
  reader->Restore(&kind, sizeof(kind));
  ::Smp::UInt64 objectId{};
  reader->Restore(&objectId, sizeof(objectId));

  if (kind != expectedKind || objectId != expectedId) {
    ::Xsmp::Exception::throwCannotRestore(
        obj, "The stored state does not belong to this object.");
  }
}

/// Self persistence of a component: the state it stores itself through the
/// ::Smp::IPersist interface.
void storeSelf(::Smp::IComponent *component, ::Smp::IStorageWriter *writer) {
  if (auto *persist = dynamic_cast<::Smp::IPersist *>(component)) {
    storeEntry(writer, PersistKind::PERSIST, identity(component));
    persist->Store(writer);
  }
}

/// External persistence of a component: the state fields it published to the
/// simulation environment. A field that is not part of the state vector stores
/// nothing.
void storeFields(const ::Smp::IComponent *component,
                 ::Smp::IStorageWriter *writer) {
  if (const auto *fields = component->GetFields()) {
    for (auto *field : *fields) {
      storeEntry(writer, PersistKind::FIELD, identity(field));
      field->Store(writer);
    }
  }
}

void restoreSelf(::Smp::IComponent *component, ::Smp::IStorageReader *reader) {
  if (auto *persist = dynamic_cast<::Smp::IPersist *>(component)) {
    check(component, reader, PersistKind::PERSIST, identity(component));
    persist->Restore(reader);
  }
}

void restoreFields(const ::Smp::IComponent *component,
                   ::Smp::IStorageReader *reader) {
  if (const auto *fields = component->GetFields()) {
    for (auto *field : *fields) {
      check(field, reader, PersistKind::FIELD, identity(field));
      field->Restore(reader);
    }
  }
}
} // namespace
// the state vector holds the published fields; the components that persist
// themselves write in a separate file, as the two are not stored in the same
// order as they are restored
static constexpr ::Smp::String8 PERSIST_FILENAME = "simulator.bin";
static constexpr ::Smp::String8 SELF_PERSIST_FILENAME = "components.bin";
/// Returns to the Standby state after an operation that left it was
/// interrupted. A failure of the event emissions is dropped: the exception
/// that interrupted the operation is the one worth reporting.
void Simulator::BackToStandby(::Smp::Services::EventId leaveEventId) noexcept {
  try {
    EmitGlobalEvent(leaveEventId);
    _state = ::Smp::SimulatorStateKind::SSK_Standby;
    EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStandbyId);
  } catch (...) {
    _state = ::Smp::SimulatorStateKind::SSK_Standby;
  }
}

void Simulator::Store(::Smp::String8 filename) {

  if (_state != ::Smp::SimulatorStateKind::SSK_Standby ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveStandbyId) {
    if (_logger) {
      _logger->Log(this,
                   "Could not Store the Simulation if simulator is not in "
                   "Standby state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    }
    return;
  }
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStandbyId);

  _state = ::Smp::SimulatorStateKind::SSK_Storing;

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStoringId);

  try {
    StorageWriter writer{filename, PERSIST_FILENAME, this};
    StorageWriter selfWriter{filename, SELF_PERSIST_FILENAME, this};

    // self persistence is performed first, so that a component can update its
    // published fields before they are stored
    recursive_action(this, [&selfWriter](::Smp::IComponent *cmp) {
      storeSelf(cmp, &selfWriter);
    });
    recursive_action(this, [&writer](const ::Smp::IComponent *cmp) {
      storeFields(cmp, &writer);
    });
  } catch (...) {
    BackToStandby(::Smp::Services::IEventManager::SMP_LeaveStoringId);
    throw;
  }

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStoringId);
  _state = ::Smp::SimulatorStateKind::SSK_Standby;

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStandbyId);
}
void Simulator::Restore(::Smp::String8 filename) {

  if (_state != ::Smp::SimulatorStateKind::SSK_Standby ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveStandbyId) {
    if (_logger) {
      _logger->Log(this,
                   "Could not Restore the Simulation if simulator is not in "
                   "Standby state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    }
    return;
  }
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStandbyId);

  _state = ::Smp::SimulatorStateKind::SSK_Restoring;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterRestoringId);
  try {
    StorageReader reader{filename, PERSIST_FILENAME, this};
    StorageReader selfReader{filename, SELF_PERSIST_FILENAME, this};

    // the published fields are restored first, so that a component can use
    // them while restoring itself
    recursive_action(this, [&reader](const ::Smp::IComponent *cmp) {
      restoreFields(cmp, &reader);
    });
    recursive_action(this, [&selfReader](::Smp::IComponent *cmp) {
      restoreSelf(cmp, &selfReader);
    });
  } catch (...) {
    BackToStandby(::Smp::Services::IEventManager::SMP_LeaveRestoringId);
    throw;
  }

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveRestoringId);
  _state = ::Smp::SimulatorStateKind::SSK_Standby;

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStandbyId);
}

void Simulator::Reconnect(::Smp::IComponent *root) {

  if (_state != ::Smp::SimulatorStateKind::SSK_Standby ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveStandbyId) {
    if (_logger) {
      _logger->Log(this,
                   "Could not Reconnect the Simulation if simulator is not in "
                   "Standby state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    }
    return;
  }
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStandbyId);
  _state = ::Smp::SimulatorStateKind::SSK_Reconnecting;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterReconnectingId);

  try {
    if (auto const *composite = dynamic_cast<::Smp::IComposite *>(root)) {
      recursive_action(composite, [this](::Smp::IComponent *cmp) {
        if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Created) {
          cmp->Publish(CreatePublication(cmp));
        }
        if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Publishing) {
          cmp->Configure(_logger, _linkRegistry);
        }
        if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Configured) {
          cmp->Connect(this);
        }
      });
    }
  } catch (...) {
    BackToStandby(::Smp::Services::IEventManager::SMP_LeaveReconnectingId);
    throw;
  }
  _state = ::Smp::SimulatorStateKind::SSK_Standby;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterStandbyId);
}

void Simulator::Exit() {

  if (_state != ::Smp::SimulatorStateKind::SSK_Standby ||
      _lastGlobalEventId ==
          ::Smp::Services::IEventManager::SMP_LeaveStandbyId) {
    if (_logger) {
      _logger->Log(this,
                   "Could not Exit from Simulation while simulator is not in "
                   "Standby state.",
                   ::Smp::Services::ILogger::LMK_Warning);
    }
    return;
  }

  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_LeaveStandbyId);
  _state = ::Smp::SimulatorStateKind::SSK_Exiting;
  EmitGlobalEvent(::Smp::Services::IEventManager::SMP_EnterExitingId);

  // Disconnect all components properly
  recursive_action(this, [](::Smp::IComponent *cmp) {
    if (cmp->GetState() == ::Smp::ComponentStateKind::CSK_Connected) {
      cmp->Disconnect();
    }
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
    if (_logger) {
      _logger->Log(entryPoint,
                   "Unable to Add an Init EntryPoint in wrong Simulator State.",
                   ::Smp::Services::ILogger::LMK_Warning);
    }
    break;
  }
}

void Simulator::AddModel(::Smp::IModel *model) {

  switch (_state) {
  case ::Smp::SimulatorStateKind::SSK_Building:
  case ::Smp::SimulatorStateKind::SSK_Connecting:
  case ::Smp::SimulatorStateKind::SSK_Initialising:
  case ::Smp::SimulatorStateKind::SSK_Standby:
    _models.AddComponent(model);
    break;
  default:
    ::Xsmp::Exception::throwInvalidSimulatorState(this, _state);
  }
}

void Simulator::AddService(::Smp::IService *service) {
  if (_state != ::Smp::SimulatorStateKind::SSK_Building) {
    ::Xsmp::Exception::throwInvalidSimulatorState(this, _state);
  }
  _services.AddComponent(service);
  /// Helper that register a standard service in the simulator
  auto RegisterService = [service, this](auto *&value) {
    if (value) {
      return;
    }
    value = dynamic_cast<std::remove_reference_t<decltype(value)>>(service);
    if (value && _logger) {
      _logger->Log(
          this,
          ("Service '" + ::Xsmp::Helper::TypeName(value) + "' registered.")
              .c_str(),
          ::Smp::Services::ILogger::LMK_Information);
    }
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

  if (!libraryPath) {
    ::Xsmp::Exception::throwLibraryNotFound(this, "", "No library name given.");
  }
  if (_logger) {
    _logger->Log(
        this,
        ("Loading '" + std::string(libraryPath) + "' library ...").c_str(),
        ::Smp::Services::ILogger::LMK_Debug);
  }
  void *handle = ::Xsmp::LoadLibrary(libraryPath);

  if (!handle) {
    auto error = ::Xsmp::GetLastError();
    if (_logger) {
      _logger->Log(this, error.c_str(), ::Smp::Services::ILogger::LMK_Error);
    }
    ::Xsmp::Exception::throwLibraryNotFound(this, libraryPath, error);
  }

  auto initialise = ::Xsmp::GetSymbol<bool (*)(
      ::Smp::ISimulator *, ::Smp::Publication::ITypeRegistry *)>(
      handle, initialiseSymbol);

  if (!initialise) {
    const std::string msg =
        std::string("Library '") + libraryPath +
        "' does not provide function 'bool Initialize(::Smp::ISimulator *, "
        "::Smp::Publication::ITypeRegistry *)': " +
        ::Xsmp::GetLastError();
    if (_logger) {
      _logger->Log(this, msg.c_str(), ::Smp::Services::ILogger::LMK_Error);
    }
    ::Xsmp::CloseLibrary(handle);
    ::Xsmp::Exception::throwInvalidLibrary(this, libraryPath, msg);
  }

  // check that Finalise exist
  if (!::Xsmp::GetSymbol<bool (*)(::Smp::ISimulator *)>(handle,
                                                        finaliseSymbol)) {
    const std::string msg = std::string("Library '") + libraryPath +
                            "' does not provide function 'bool Finalise()': " +
                            ::Xsmp::GetLastError();
    if (_logger) {
      _logger->Log(this, msg.c_str(), ::Smp::Services::ILogger::LMK_Error);
    }
    ::Xsmp::CloseLibrary(handle);
    ::Xsmp::Exception::throwInvalidLibrary(this, libraryPath, msg);
  }

  bool initialised = false;
  try {
    initialised = (*initialise)(this, &_typeRegistry);
  } catch (const std::exception &e) {
    // Initialise() reports a failure through its return value: an exception
    // is outside this operation's contract and would leave the library loaded
    const std::string msg = std::string("Initialise() of library '") +
                            libraryPath + "' threw: " + e.what();
    if (_logger) {
      _logger->Log(this, msg.c_str(), ::Smp::Services::ILogger::LMK_Error);
    }
    ::Xsmp::CloseLibrary(handle);
    ::Xsmp::Exception::throwInvalidLibrary(this, libraryPath, msg);
  } catch (...) {
    const std::string msg = std::string("Initialise() of library '") +
                            libraryPath + "' threw an unknown exception.";
    if (_logger) {
      _logger->Log(this, msg.c_str(), ::Smp::Services::ILogger::LMK_Error);
    }
    ::Xsmp::CloseLibrary(handle);
    ::Xsmp::Exception::throwInvalidLibrary(this, libraryPath, msg);
  }

  if (initialised) {
    if (_logger) {
      _logger->Log(
          this,
          ("Library '" + std::string(libraryPath) + "' successfully loaded.")
              .c_str(),
          ::Smp::Services::ILogger::LMK_Debug);
    }
  } else {
    const std::string msg =
        std::string("Initialise() of library '") + libraryPath + "' failed.";
    if (_logger) {
      _logger->Log(this, msg.c_str(), ::Smp::Services::ILogger::LMK_Error);
    }
    ::Xsmp::CloseLibrary(handle);
    ::Xsmp::Exception::throwInvalidLibrary(this, libraryPath, msg);
  }
  _libraries.emplace_back(libraryPath, handle);
}

} // namespace Xsmp
