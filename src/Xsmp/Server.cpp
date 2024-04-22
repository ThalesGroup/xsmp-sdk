// Copyright 2024 THALES ALENIA SPACE FRANCE. All rights reserved.
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

#include <Smp/AccessKind.h>
#include <Smp/AnySimple.h>
#include <Smp/ComponentStateKind.h>
#include <Smp/Exception.h>
#include <Smp/IAggregate.h>
#include <Smp/IArrayField.h>
#include <Smp/ICollection.h>
#include <Smp/IContainer.h>
#include <Smp/IDataflowField.h>
#include <Smp/IDynamicInvocation.h>
#include <Smp/IEntryPoint.h>
#include <Smp/IEntryPointPublisher.h>
#include <Smp/IEventConsumer.h>
#include <Smp/IEventProvider.h>
#include <Smp/IEventSink.h>
#include <Smp/IEventSource.h>
#include <Smp/IFactory.h>
#include <Smp/IFailure.h>
#include <Smp/IFallibleModel.h>
#include <Smp/IForcibleField.h>
#include <Smp/ILinkingComponent.h>
#include <Smp/IOperation.h>
#include <Smp/IParameter.h>
#include <Smp/IProperty.h>
#include <Smp/IReference.h>
#include <Smp/IRequest.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimulator.h>
#include <Smp/IStructureField.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Publication/IArrayType.h>
#include <Smp/Publication/ParameterDirectionKind.h>
#include <Smp/Services/IEventManager.h>
#include <Smp/Services/ILinkRegistry.h>
#include <Smp/Services/ILogger.h>
#include <Smp/Services/IResolver.h>
#include <Smp/Services/IScheduler.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Smp/SimulatorStateKind.h>
#include <Smp/Uuid.h>
#include <Smp/ViewKind.h>
#include <Xsmp/Exception.h>
#include <Xsmp/Helper.h>
#include <Xsmp/LibraryHelper.h>
#include <algorithm>
#include <crow.h>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iterator>
#include <map>
#include <memory>
#include <sstream>
#include <stddef.h>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <typeindex>
#include <typeinfo>
#include <unordered_map>
#include <utility>
#include <vector>

crow::json::wvalue save(const Smp::AnySimple &value) {
  switch (value.type) {
  case Smp::PrimitiveTypeKind::PTK_Bool:
    return static_cast<Smp::Bool>(value);
  case Smp::PrimitiveTypeKind::PTK_Char8:
    return static_cast<Smp::Char8>(value);
  case Smp::PrimitiveTypeKind::PTK_DateTime:
    return static_cast<Smp::DateTime>(value);
  case Smp::PrimitiveTypeKind::PTK_Duration:
    return static_cast<Smp::Duration>(value);
  case Smp::PrimitiveTypeKind::PTK_Float32:
    return static_cast<Smp::Float32>(value);
  case Smp::PrimitiveTypeKind::PTK_Float64:
    return static_cast<Smp::Float64>(value);
  case Smp::PrimitiveTypeKind::PTK_Int16:
    return static_cast<Smp::Int16>(value);
  case Smp::PrimitiveTypeKind::PTK_Int32:
    return static_cast<Smp::Int32>(value);
  case Smp::PrimitiveTypeKind::PTK_Int64:
    return static_cast<Smp::Int64>(value);
  case Smp::PrimitiveTypeKind::PTK_Int8:
    return static_cast<Smp::Int8>(value);
  case Smp::PrimitiveTypeKind::PTK_String8:
    return static_cast<Smp::String8>(value);
  case Smp::PrimitiveTypeKind::PTK_UInt16:
    return static_cast<Smp::UInt16>(value);
  case Smp::PrimitiveTypeKind::PTK_UInt32:
    return static_cast<Smp::UInt32>(value);
  case Smp::PrimitiveTypeKind::PTK_UInt64:
    return static_cast<Smp::UInt64>(value);
  case Smp::PrimitiveTypeKind::PTK_UInt8:
    return static_cast<Smp::UInt8>(value);
  default:
    return crow::json::wvalue::empty_object();
  }
}

template <typename T>
inline std::enable_if_t<std::is_base_of_v<Smp::IObject, T>, crow::json::wvalue>
save(const T *value) {
  return value ? Xsmp::Helper::GetPath(value) : crow::json::wvalue{};
}

template <typename T>
inline std::enable_if_t<std::is_arithmetic_v<T>, crow::json::wvalue>
save(T value) {
  return value;
}
inline crow::json::wvalue save(Smp::String8 str) { return str; }

template <typename T>
inline std::enable_if_t<std::is_enum_v<T>, crow::json::wvalue> save(T value) {
  std::stringstream ss;
  ss << value;
  return ss.str();
}
inline crow::json::wvalue save(const Smp::Uuid &uuid) {
  std::stringstream ss;
  ss << uuid;
  return ss.str();
}

template <typename T>
inline crow::json::wvalue save(const Smp::ICollection<T> *collection);
inline crow::json::wvalue save(const Smp::IContainer *container) {
  crow::json::wvalue value;
  if (container) {
    value["name"] = container->GetName();
    value["description"] = container->GetDescription();
    value["lower"] = container->GetLower();
    value["upper"] = container->GetUpper();
    value["count"] = container->GetCount();
    value["components"] = save(container->GetComponents());
    value["cpp_type"] = Xsmp::Helper::TypeName(container);
  }
  return value;
}
inline crow::json::wvalue save(const Smp::IReference *reference) {
  crow::json::wvalue value;
  if (reference) {
    value["name"] = reference->GetName();
    value["description"] = reference->GetDescription();
    value["lower"] = reference->GetLower();
    value["upper"] = reference->GetUpper();
    value["count"] = reference->GetCount();
    value["components"] = save(reference->GetComponents());
    value["cpp_type"] = Xsmp::Helper::TypeName(reference);
  }
  return value;
}
inline crow::json::wvalue save(const Smp::IFactory *factory) {
  crow::json::wvalue value;
  if (factory) {
    value["name"] = factory->GetName();
    value["description"] = factory->GetDescription();
    value["uuid"] = save(factory->GetUuid());
    value["type_name"] = factory->GetTypeName();
  }
  return value;
}

template <typename T>
inline crow::json::wvalue save(const Smp::ICollection<T> *collection) {
  crow::json::wvalue::list list;
  if (collection) {
    for (auto *elem : *collection) {
      list.emplace_back(save(elem));
    }
  }
  return list;
}

inline crow::json::wvalue save(const Smp::Publication::IType *type) {
  crow::json::wvalue value;
  if (type) {

    value["name"] = type->GetName();
    value["description"] = type->GetDescription();
    value["primitive_type_kind"] = save(type->GetPrimitiveTypeKind());
    value["uuid"] = save(type->GetUuid());
    if (const auto *arrayType =
            dynamic_cast<const Smp::Publication::IArrayType *>(type)) {
      value["size"] = arrayType->GetSize();
      value["item_type"] = save(arrayType->GetItemType());
    }
  }
  return value;
}
inline crow::json::wvalue save(const Smp::IParameter *parameter) {
  crow::json::wvalue value;
  if (parameter) {
    value["name"] = parameter->GetName();
    value["description"] = parameter->GetDescription();
    value["direction"] = save(parameter->GetDirection());
    value["type"] = save(parameter->GetType());
  }
  return value;
}

template <typename T> struct arg_v;
struct arg {
  constexpr explicit arg(const char *_name) : name(_name) {}
  constexpr explicit arg(const arg &) = default;
  template <typename T> constexpr arg_v<T> operator=(T &&value) const;
  const char *name;
};
template <typename T> struct arg_v : arg {

public:
  constexpr arg_v(const arg &_arg, T &&x) : arg(_arg), value{std::move(x)} {}
  /// The default value
  T value;
};
template <typename T> constexpr arg_v<T> arg::operator=(T &&value) const {
  return {*this, std::forward<T>(value)};
}

template <typename T> T load(const char *value);

// Specialization for Smp::String8
template <> Smp::String8 load<Smp::String8>(const char *value) { return value; }

// Specialization for unsigned long long int
template <>
unsigned long long int load<unsigned long long int>(const char *value) {
  char *endptr;
  auto result = std::strtoull(value, &endptr, 10);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return result;
}
// Specialization for long long int
template <> long long int load<long long int>(const char *value) {
  char *endptr;
  auto result = std::strtoll(value, &endptr, 10);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return result;
}

// Specialization for unsigned long int
template <> unsigned long int load<unsigned long int>(const char *value) {
  char *endptr;
  auto result = std::strtoul(value, &endptr, 10);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return result;
}
// Specialization for  long int
template <> long int load<long int>(const char *value) {
  char *endptr;
  auto result = std::strtol(value, &endptr, 10);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return result;
}
template <typename T, typename U> T checkRange(U value) {
  if (value < std::numeric_limits<T>::min() ||
      value > std::numeric_limits<T>::max()) {
    throw std::invalid_argument(
        "Conversion failed: the number '" + std::to_string(value) +
        "' is not convertible to " + typeid(T).name() + ".");
  }
  return static_cast<T>(value);
}

// Specialization for unsigned int
template <> unsigned int load<unsigned int>(const char *value) {
  char *endptr;
  auto result = std::strtoul(value, &endptr, 10);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return checkRange<unsigned int>(result);
}
// Specialization for int
template <> int load<int>(const char *value) {
  char *endptr;
  auto result = std::strtol(value, &endptr, 10);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return checkRange<int>(result);
}

// Specialization for unsigned short
template <> unsigned short load<unsigned short>(const char *value) {
  char *endptr;
  auto result = std::strtoul(value, &endptr, 10);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return checkRange<unsigned short>(result);
}
// Specialization for short
template <> short load<short>(const char *value) {
  char *endptr;
  auto result = std::strtol(value, &endptr, 10);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return checkRange<short>(result);
}

// Specialization for unsigned char
template <> unsigned char load<unsigned char>(const char *value) {
  char *endptr;
  auto result = std::strtoul(value, &endptr, 10);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return checkRange<unsigned char>(result);
}
// Specialization for signed char
template <> signed char load<signed char>(const char *value) {
  char *endptr;
  auto result = std::strtol(value, &endptr, 10);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return checkRange<signed char>(result);
}
// Specialization for char
template <> char load<char>(const char *value) {
  char *endptr;
  auto result = std::strtol(value, &endptr, 10);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return checkRange<char>(result);
}
// Specialization for float
template <> float load<float>(const char *value) {
  char *endptr;
  auto result = std::strtof(value, &endptr);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return result;
}

// Specialization for double
template <> double load<double>(const char *value) {
  char *endptr;
  auto result = std::strtod(value, &endptr);
  if (*endptr != '\0') {
    throw std::invalid_argument("Conversion failed: the string is not a valid "
                                "number: unexpected characters '" +
                                std::string(endptr) + "'.");
  }
  return result;
}

// Specialization for Bool
template <> Smp::Bool load<Smp::Bool>(const char *value) {
  return strcmp("true", value) == 0;
}

// Specialization for Smp::Uuid
template <> Smp::Uuid load<Smp::Uuid>(const char *value) {
  return Smp::Uuid{value};
}

[[noreturn]] void throwMissingArgument(const char *name) {
  throw std::invalid_argument("The argument '" + std::string(name) +
                              "' is missing from the request.");
}
[[noreturn]] void throwConversionError(const Smp::IObject *object,
                                       Smp::PrimitiveTypeKind kind,
                                       const char *value,
                                       const std::exception &e) {

  Xsmp::Exception::throwException(object, "conversion error", "",
                                  "Unable to load '", std::string(value),
                                  "' to '", kind, "': ", e.what());
}

template <typename T>
[[noreturn]] void throwConversionError(const char *name, const char *value,
                                       const std::exception &e) {
  Xsmp::Exception::throwException(
      nullptr, "conversion error", "", "Unable to convert '", value, "' to '",
      Xsmp::Helper::TypeName<T>(), "' for argument '", name, "': ", e.what());
}
template <typename T> T Resolve(::Smp::ISimulator *sim, const char *value) {

  auto *obj = Xsmp::Helper::Resolve(sim, value);
  if (!obj) {
    throw std::invalid_argument("Cannot resolve the object.");
  }
  auto casted = dynamic_cast<T>(obj);
  if (!casted) {
    throw std::invalid_argument(Xsmp::Helper::TypeName(obj) +
                                " is not convertible to " +
                                Xsmp::Helper::TypeName<T>());
  }

  return casted;
}
template <typename T>
T extractArgAndConvert(::Smp::ISimulator *sim, const crow::query_string &query,
                       const arg &arg) {
  if (auto *value = query.get(arg.name)) {
    try {
      if constexpr (std::is_base_of_v<::Smp::IObject,
                                      std::remove_pointer_t<T>>) {
        return Resolve<T>(sim, value);
      } else {
        return load<T>(value);
      }
    } catch (std::exception &e) {
      throwConversionError<T>(arg.name, value, e);
    }
  }
  throwMissingArgument(arg.name);
}
template <typename T, typename U>
T extractArgAndConvert(::Smp::ISimulator *sim, const crow::query_string &query,
                       const arg_v<U> &arg) {
  if (auto *value = query.get(arg.name)) {
    try {
      if constexpr (std::is_base_of_v<::Smp::IObject,
                                      std::remove_pointer_t<T>>) {
        return Resolve<T>(sim, value);
      } else {
        return load<T>(value);
      }
    } catch (std::exception &e) {
      throwConversionError<T>(arg.name, value, e);
    }
  }
  return arg.value;
}

void checkArg(crow::query_string &query, const arg &arg) {
  if (!query.pop(arg.name)) {
    throwMissingArgument(arg.name);
  }
}
template <typename U>
void checkArg(crow::query_string &query, const arg_v<U> &arg) {
  // the arg is optional but pop the arg from the query
  query.pop(arg.name);
}

template <typename... ArgsName>
void checkArgs(crow::query_string query, const ArgsName &...args) {
  (checkArg(query, args), ...);
  if (!query.keys().empty()) {
    std::stringstream ss;
    auto keys = query.keys();
    const auto length = keys.size();
    size_t count = 0;

    for (auto iter = keys.begin(); iter != keys.end(); ++iter, ++count) {
      ss << *iter;
      if (count < length - 1) {
        ss << ", ";
      }
    }
    throw std::invalid_argument("unexpected argument(s): " + ss.str());
  }
}

Smp::AnySimple load(Smp::PrimitiveTypeKind kind, const char *value) {

  switch (kind) {
  case Smp::PrimitiveTypeKind::PTK_Bool:
    return {kind, load<Smp::Bool>(value)};
  case Smp::PrimitiveTypeKind::PTK_Char8:
    return {kind, load<Smp::Char8>(value)};
  case Smp::PrimitiveTypeKind::PTK_DateTime:
    return {kind, load<Smp::DateTime>(value)};
  case Smp::PrimitiveTypeKind::PTK_Duration:
    return {kind, load<Smp::Duration>(value)};
  case Smp::PrimitiveTypeKind::PTK_Float32:
    return {kind, load<Smp::Float32>(value)};
  case Smp::PrimitiveTypeKind::PTK_Float64:
    return {kind, load<Smp::Float64>(value)};
  case Smp::PrimitiveTypeKind::PTK_Int16:
    return {kind, load<Smp::Int16>(value)};
  case Smp::PrimitiveTypeKind::PTK_Int32:
    return {kind, load<Smp::Int32>(value)};
  case Smp::PrimitiveTypeKind::PTK_Int64:
    return {kind, load<Smp::Int64>(value)};
  case Smp::PrimitiveTypeKind::PTK_Int8:
    return {kind, load<Smp::Int8>(value)};
  case Smp::PrimitiveTypeKind::PTK_String8:
    return {kind, load<Smp::String8>(value)};
  case Smp::PrimitiveTypeKind::PTK_UInt16:
    return {kind, load<Smp::UInt16>(value)};
  case Smp::PrimitiveTypeKind::PTK_UInt32:
    return {kind, load<Smp::UInt32>(value)};
  case Smp::PrimitiveTypeKind::PTK_UInt64:
    return {kind, load<Smp::UInt64>(value)};
  case Smp::PrimitiveTypeKind::PTK_UInt8:
    return {kind, load<Smp::UInt8>(value)};
  default:
    return {};
  }
}

void CreateInstance(Smp::ISimulator &sim, Smp::IComposite *parent,
                    Smp::Uuid uuid, Smp::String8 name, Smp::String8 container,
                    Smp::String8 description) {
  if (!parent) {
    Xsmp::Exception::throwException(
        nullptr, "parent is null",
        "cannot create the instance because the parent is null.");
  }
  auto *ctn = parent->GetContainer(container);
  if (!ctn) {
    Xsmp::Exception::throwException(nullptr, "no container",
                                    "Container '" + std::string(container) +
                                        "' does not exist.");
  }
  auto instance = std::unique_ptr<Smp::IComponent>(
      sim.CreateInstance(uuid, name, description, parent));
  if (!instance) {
    Xsmp::Exception::throwException(
        nullptr, "no factory",
        "no factory for the given uuid has been registered");
  }
  ctn->AddComponent(instance.get());
  // release the pointer if the instance was successfully added
  instance.release();
}

void SetFieldValue(Smp::ISimpleField &field, const char *value) {
  auto kind = field.GetPrimitiveTypeKind();
  try {
    field.SetValue(load(kind, value));
  } catch (const Smp::Exception &) {
    throw;
  } catch (const std::exception &e) {
    throwConversionError(&field, kind, value, e);
  }
}

void ForceFieldValue(Smp::IForcibleField &field, const char *value) {
  auto kind = field.GetPrimitiveTypeKind();
  try {
    field.Force(load(kind, value));
  } catch (const Smp::Exception &) {
    throw;
  } catch (const std::exception &e) {
    throwConversionError(&field, kind, value, e);
  }
}
void SetSimpleArrayFieldValue(Smp::ISimpleArrayField &field, Smp::UInt64 index,
                              const char *value) {
  auto kind =
      dynamic_cast<const Smp::Publication::IArrayType *>(field.GetType())
          ->GetItemType()
          ->GetPrimitiveTypeKind();
  try {
    field.SetValue(index, load(kind, value));
  } catch (const Smp::Exception &) {
    // re-throw Smp exception directly
    throw;
  } catch (const std::exception &e) {
    throwConversionError(&field, kind, value, e);
  }
}

void SetPropertyValue(Smp::IProperty &property, const char *value) {
  auto kind = property.GetType()->GetPrimitiveTypeKind();
  try {
    property.SetValue(load(property.GetType()->GetPrimitiveTypeKind(), value));
  } catch (const Smp::Exception &) {
    // re-throw Smp exception directly
    throw;
  } catch (const std::exception &e) {
    throwConversionError(&property, kind, value, e);
  }
}

void DeleteComponent(Smp::IComponent &component) {

  if (const auto *parent =
          dynamic_cast<const Smp::IComposite *>(component.GetParent())) {
    for (auto *container : *parent->GetContainers()) {
      if (container->GetComponent(component.GetName())) {
        container->DeleteComponent(&component);
        return;
      }
    }
  }
  Xsmp::Exception::throwException(
      &component, "cannot be deleted",
      "The component is not contained in its parent.");
}

void LogDebug(Smp::Services::ILogger &logger, const Smp::IObject *sender,
              Smp::String8 message) {
  logger.Log(sender, message, Smp::Services::ILogger::LMK_Debug);
}
void LogInfo(Smp::Services::ILogger &logger, const Smp::IObject *sender,
             Smp::String8 message) {
  logger.Log(sender, message, Smp::Services::ILogger::LMK_Information);
}
void LogWarning(Smp::Services::ILogger &logger, const Smp::IObject *sender,
                Smp::String8 message) {
  logger.Log(sender, message, Smp::Services::ILogger::LMK_Warning);
}
void LogError(Smp::Services::ILogger &logger, const Smp::IObject *sender,
              Smp::String8 message) {
  logger.Log(sender, message, Smp::Services::ILogger::LMK_Error);
}
void LogEvent(Smp::Services::ILogger &logger, const Smp::IObject *sender,
              Smp::String8 message) {
  logger.Log(sender, message, Smp::Services::ILogger::LMK_Event);
}
void Log(Smp::Services::ILogger &logger, const Smp::IObject *sender,
         Smp::String8 message, Smp::String8 kind) {
  logger.Log(sender, message, logger.QueryLogMessageKind(kind));
}
void NotifyEventSink(Smp::IEventSink &eventSink, Smp::IObject *sender,
                     const char *arg) {
  eventSink.Notify(sender, load(eventSink.GetEventArgType(), arg));
}

class type {
public:
  type(Smp::IObject *object) {

    using namespace crow::json;
    wvalue::list interfaces;
    interfaces.emplace_back("Smp::IObject");

    def("GetName", &Smp::IObject::GetName);
    def("GetDescription", &Smp::IObject::GetDescription);
    def("GetParent", &Smp::IObject::GetParent);

    if (dynamic_cast<Smp::IContainer *>(object)) {
      interfaces.emplace_back("Smp::IContainer");
      attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
        const auto *container = dynamic_cast<const Smp::IContainer *>(obj);
        value["lower"] = container->GetLower();
        value["upper"] = container->GetUpper();
        value["count"] = container->GetCount();
        value["components"] = save(container->GetComponents());
      });

      def("GetComponents", &Smp::IContainer::GetComponents);
      def("GetComponent", &Smp::IContainer::GetComponent, arg("name"));
      def("GetCount", &Smp::IContainer::GetCount);
      def("GetLower", &Smp::IContainer::GetLower);
      def("GetUpper", &Smp::IContainer::GetUpper);
      def("DeleteComponent", &Smp::IContainer::DeleteComponent,
          arg("component"));
    }
    if (dynamic_cast<Smp::IReference *>(object)) {
      interfaces.emplace_back("Smp::IReference");
      attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
        const auto *reference = dynamic_cast<const Smp::IReference *>(obj);
        value["lower"] = reference->GetLower();
        value["upper"] = reference->GetUpper();
        value["count"] = reference->GetCount();
        value["components"] = save(reference->GetComponents());
      });
      def("GetComponents", &Smp::IReference::GetComponents);
      def("GetComponent", &Smp::IReference::GetComponent, arg("name"));
      def("GetCount", &Smp::IReference::GetCount);
      def("GetLower", &Smp::IReference::GetLower);
      def("GetUpper", &Smp::IReference::GetUpper);
      def("AddComponent", &Smp::IReference::AddComponent, arg("component"));
      def("RemoveComponent", &Smp::IReference::RemoveComponent,
          arg("component"));
    }
    if (dynamic_cast<Smp::IComposite *>(object)) {

      interfaces.emplace_back("Smp::IComposite");
      attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
        const auto *composite = dynamic_cast<const Smp::IComposite *>(obj);
        value["containers"] = save(composite->GetContainers());
      });
      def("GetContainers", &Smp::IComposite::GetContainers);
      def("GetContainer", &Smp::IComposite::GetContainer, arg("name"));

      if (dynamic_cast<Smp::ISimulator *>(object)) {
        interfaces.emplace_back("Smp::ISimulator");
        attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
          const auto *sim = dynamic_cast<const Smp::ISimulator *>(obj);
          value["state"] = save(sim->GetState());
        });
        def("Abort", &Smp::ISimulator::Abort);
        def("AddInitEntryPoint", &Smp::ISimulator::AddInitEntryPoint,
            arg("entryPoint"));
        def("Configure", &Smp::ISimulator::Configure);
        def("Connect", &Smp::ISimulator::Connect);
        def("Exit", &Smp::ISimulator::Exit);
        def("GetEventManager", &Smp::ISimulator::GetEventManager);
        def("GetLinkRegistry", &Smp::ISimulator::GetLinkRegistry);
        def("GetLogger", &Smp::ISimulator::GetLogger);
        def("GetResolver", &Smp::ISimulator::GetResolver);
        def("GetScheduler", &Smp::ISimulator::GetScheduler);
        def("GetService", &Smp::ISimulator::GetService, arg("name"));
        def("GetState", &Smp::ISimulator::GetState);
        def("GetTimeKeeper", &Smp::ISimulator::GetTimeKeeper);
        def("Hold", &Smp::ISimulator::Hold, arg("immediate"));
        def("Initialise", &Smp::ISimulator::Initialise);
        def("Publish", &Smp::ISimulator::Publish);
        def("Reconnect", &Smp::ISimulator::Reconnect, arg("root"));
        def("Run", &Smp::ISimulator::Run); // TODO handle duration ?
        def("GetFactory", &Smp::ISimulator::GetFactory, arg("uuid"));
        def("GetFactories", &Smp::ISimulator::GetFactories);
        def("Restore", &Smp::ISimulator::Restore, arg("filename"));
        def("Store", &Smp::ISimulator::Store, arg("filename"));
        def("LoadLibrary", &Smp::ISimulator::LoadLibrary, arg("name"));
        def("CreateInstance", &CreateInstance, arg("parent"), arg("uuid"),
            arg("name"), arg("container"), arg("description") = "");
      }
    }

    if (dynamic_cast<Smp::IComponent *>(object)) {
      interfaces.emplace_back("Smp::IComponent");
      attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
        const auto *component = dynamic_cast<const Smp::IComponent *>(obj);
        value["fields"] = save(component->GetFields());
        value["state"] = save(component->GetState());
        value["uuid"] = save(component->GetUuid());
      });

      def("GetFields", &Smp::IComponent::GetFields);
      def("GetState", &Smp::IComponent::GetState);
      def("GetUuid", &Smp::IComponent::GetUuid);
      def("Delete", &DeleteComponent);

      if (dynamic_cast<Smp::IAggregate *>(object)) {
        interfaces.emplace_back("Smp::IAggregate");
        def("GetReferences", &Smp::IAggregate::GetReferences);
        def("GetReference", &Smp::IAggregate::GetReference, arg("name"));

        attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
          const auto *aggregate = dynamic_cast<const Smp::IAggregate *>(obj);
          value["references"] = save(aggregate->GetReferences());
        });
      }

      if (dynamic_cast<Smp::IEventProvider *>(object)) {
        interfaces.emplace_back("Smp::IEventProvider");
        def("GetEventSources", &Smp::IEventProvider::GetEventSources);
        def("GetEventSource", &Smp::IEventProvider::GetEventSource,
            arg("name"));
        attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
          const auto *eventProvider =
              dynamic_cast<const Smp::IEventProvider *>(obj);
          value["event_sources"] = save(eventProvider->GetEventSources());
        });
      }
      if (dynamic_cast<Smp::IEventConsumer *>(object)) {
        interfaces.emplace_back("Smp::IEventConsumer");
        def("GetEventSinks", &Smp::IEventConsumer::GetEventSinks);
        def("GetEventSink", &Smp::IEventConsumer::GetEventSink, arg("name"));
        attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
          const auto *eventConsumer =
              dynamic_cast<const Smp::IEventConsumer *>(obj);
          value["event_sinks"] = save(eventConsumer->GetEventSinks());
        });
      }

      if (dynamic_cast<Smp::IDynamicInvocation *>(object)) {
        interfaces.emplace_back("Smp::IDynamicInvocation");
        def("GetOperations", &Smp::IDynamicInvocation::GetOperations);
        def("GetProperties", &Smp::IDynamicInvocation::GetProperties);
        attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
          const auto *dynamicInvocation =
              dynamic_cast<const Smp::IDynamicInvocation *>(obj);
          value["operations"] = save(dynamicInvocation->GetOperations());
          value["properties"] = save(dynamicInvocation->GetProperties());
        });
      }
      if (dynamic_cast<Smp::ILinkingComponent *>(object)) {
        interfaces.emplace_back("Smp::ILinkingComponent");
        def("RemoveLinks", &Smp::ILinkingComponent::RemoveLinks, arg("target"));
      }
      if (dynamic_cast<Smp::IModel *>(object)) {
        interfaces.emplace_back("Smp::IModel");
        if (dynamic_cast<Smp::IFallibleModel *>(object)) {
          interfaces.emplace_back("Smp::IFallibleModel");
          def("IsFailed", &Smp::IFallibleModel::IsFailed);
          def("GetFailures", &Smp::IFallibleModel::GetFailures);
          def("GetFailure", &Smp::IFallibleModel::GetFailure, arg("name"));
          attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
            const auto *fallibleModel =
                dynamic_cast<const Smp::IFallibleModel *>(obj);
            value["failed"] = fallibleModel->IsFailed();
            value["failures"] = save(fallibleModel->GetFailures());
          });
        }
      }
      if (dynamic_cast<Smp::IService *>(object)) {
        interfaces.emplace_back("Smp::IService");

        if (dynamic_cast<Smp::Services::IEventManager *>(object)) {
          interfaces.emplace_back("Smp::Services::IEventManager");
          def("QueryEventId", &Smp::Services::IEventManager::QueryEventId,
              arg("eventName"));
          def("Emit", &Smp::Services::IEventManager::Emit, arg("event"),
              arg("synchronous") = true);
          def("Subscribe", &Smp::Services::IEventManager::Subscribe,
              arg("event"), arg("entryPoint"));
          def("Unsubscribe", &Smp::Services::IEventManager::Unsubscribe,
              arg("event"), arg("entryPoint"));
        }

        if (dynamic_cast<Smp::Services::ILinkRegistry *>(object)) {
          interfaces.emplace_back("Smp::Services::ILinkRegistry");
          def("AddLink", &Smp::Services::ILinkRegistry::AddLink, arg("source"),
              arg("target"));
          def("CanRemove", &Smp::Services::ILinkRegistry::CanRemove,
              arg("target"));
          def("GetLinkCount", &Smp::Services::ILinkRegistry::GetLinkCount,
              arg("source"), arg("target"));
          def("GetLinkCount", &Smp::Services::ILinkRegistry::GetLinkSources,
              arg("target"));
          def("RemoveLink", &Smp::Services::ILinkRegistry::RemoveLink,
              arg("source"), arg("target"));
          def("RemoveLinks", &Smp::Services::ILinkRegistry::RemoveLinks,
              arg("target"));
        }
        if (dynamic_cast<Smp::Services::ILogger *>(object)) {
          interfaces.emplace_back("Smp::Services::ILogger");
          def("LogDebug", &LogDebug, arg("sender") = nullptr, arg("message"));
          def("LogInfo", &LogInfo, arg("sender") = nullptr, arg("message"));
          def("LogWarning", &LogWarning, arg("sender") = nullptr,
              arg("message"));
          def("LogError", &LogError, arg("sender") = nullptr, arg("message"));
          def("LogEvent", &LogEvent, arg("sender") = nullptr, arg("message"));
          def("Log", &Log, arg("sender") = nullptr, arg("message"),
              arg("kind"));
        }
        if (dynamic_cast<Smp::Services::IResolver *>(object)) {
          interfaces.emplace_back("Smp::Services::IResolver");
          def("ResolveAbsolute", &Smp::Services::IResolver::ResolveAbsolute,
              arg("absolutePath"));
          def("ResolveRelative", &Smp::Services::IResolver::ResolveRelative,
              arg("relativePath"), arg("sender"));
        }
        if (dynamic_cast<Smp::Services::IScheduler *>(object)) {
          interfaces.emplace_back("Smp::Services::IScheduler");
          def("AddImmediateEvent",
              &Smp::Services::IScheduler::AddImmediateEvent, arg("entryPoint"));
          def("AddSimulationTimeEvent",
              &Smp::Services::IScheduler::AddSimulationTimeEvent,
              arg("entryPoint"), arg("simulationTime"),
              arg("cycleTime") = static_cast<Smp::Duration>(0),
              arg("repeat") = static_cast<Smp::Int64>(0));
          def("AddMissionTimeEvent",
              &Smp::Services::IScheduler::AddMissionTimeEvent,
              arg("entryPoint"), arg("missionTime"),
              arg("cycleTime") = static_cast<Smp::Duration>(0),
              arg("repeat") = static_cast<Smp::Int64>(0));
          def("AddEpochTimeEvent",
              &Smp::Services::IScheduler::AddEpochTimeEvent, arg("entryPoint"),
              arg("epochTime"),
              arg("cycleTime") = static_cast<Smp::Duration>(0),
              arg("repeat") = static_cast<Smp::Int64>(0));
          def("AddZuluTimeEvent", &Smp::Services::IScheduler::AddZuluTimeEvent,
              arg("entryPoint"), arg("zuluTime"),
              arg("cycleTime") = static_cast<Smp::Duration>(0),
              arg("repeat") = static_cast<Smp::Int64>(0));
          def("SetEventSimulationTime",
              &Smp::Services::IScheduler::SetEventSimulationTime, arg("event"),
              arg("simulationTime"));
          def("SetEventMissionTime",
              &Smp::Services::IScheduler::SetEventMissionTime, arg("event"),
              arg("missionTime"));
          def("SetEventEpochTime",
              &Smp::Services::IScheduler::SetEventEpochTime, arg("event"),
              arg("epochTime"));
          def("SetEventZuluTime", &Smp::Services::IScheduler::SetEventZuluTime,
              arg("event"), arg("zuluTime"));
          def("SetEventCycleTime",
              &Smp::Services::IScheduler::SetEventCycleTime, arg("event"),
              arg("cycleTime"));
          def("SetEventRepeat", &Smp::Services::IScheduler::SetEventRepeat,
              arg("event"), arg("repeat"));
          def("RemoveEvent", &Smp::Services::IScheduler::RemoveEvent,
              arg("event"));
          def("GetCurrentEventId",
              &Smp::Services::IScheduler::GetCurrentEventId);
          def("GetNextScheduledEventTime",
              &Smp::Services::IScheduler::GetNextScheduledEventTime);
        }
        if (dynamic_cast<Smp::Services::ITimeKeeper *>(object)) {
          interfaces.emplace_back("Smp::Services::ITimeKeeper");
          def("GetSimulationTime",
              &Smp::Services::ITimeKeeper::GetSimulationTime);
          def("GetEpochTime", &Smp::Services::ITimeKeeper::GetEpochTime);
          def("GetMissionStartTime",
              &Smp::Services::ITimeKeeper::GetMissionStartTime);
          def("GetMissionTime", &Smp::Services::ITimeKeeper::GetMissionTime);
          def("GetZuluTime", &Smp::Services::ITimeKeeper::GetZuluTime);
          def("SetSimulationTime",
              &Smp::Services::ITimeKeeper::SetSimulationTime,
              arg("simulationTime)"));
          def("SetEpochTime", &Smp::Services::ITimeKeeper::SetEpochTime,
              arg("epochTime)"));
          def("SetMissionStartTime",
              &Smp::Services::ITimeKeeper::SetMissionStartTime,
              arg("missionStart)"));
          def("SetMissionTime", &Smp::Services::ITimeKeeper::SetMissionTime,
              arg("missionTime)"));
        }
      }
    }
    if (dynamic_cast<Smp::IEntryPointPublisher *>(object)) {
      interfaces.emplace_back("Smp::IEntryPointPublisher");
      def("GetEntryPoints", &Smp::IEntryPointPublisher::GetEntryPoints);
      def("GetEntryPoint", &Smp::IEntryPointPublisher::GetEntryPoint,
          arg("name"));
      attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
        const auto *entryPointPublisher =
            dynamic_cast<const Smp::IEntryPointPublisher *>(obj);
        value["entry_points"] = save(entryPointPublisher->GetEntryPoints());
      });
    }
    if (dynamic_cast<Smp::IEntryPoint *>(object)) {
      interfaces.emplace_back("Smp::IEntryPoint");
      def("Execute", &Smp::IEntryPoint::Execute);
    }

    if (dynamic_cast<Smp::IEventSink *>(object)) {
      interfaces.emplace_back("Smp::IEventSink");

      def("GetEventArgType", &Smp::IEventSink::GetEventArgType);
      def("Notify", &NotifyEventSink, arg("sender"), arg("arg"));
      attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
        const auto *eventSink = dynamic_cast<const Smp::IEventSink *>(obj);
        value["event_arg_type"] = save(eventSink->GetEventArgType());
      });
    }
    if (dynamic_cast<Smp::IEventSource *>(object)) {
      interfaces.emplace_back("Smp::IEventSource");
      def("Subscribe", &Smp::IEventSource::Subscribe, arg("eventSink"));
      def("Unsubscribe", &Smp::IEventSource::Unsubscribe, arg("eventSink"));
    }
    if (dynamic_cast<Smp::IFailure *>(object)) {
      interfaces.emplace_back("Smp::IFailure");
      attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
        const auto *failure = dynamic_cast<const Smp::IFailure *>(obj);
        value["failed"] = failure->IsFailed();
      });
      def("IsFailed", &Smp::IFailure::IsFailed);
      def("Unfail", &Smp::IFailure::Unfail);
      def("Fail", &Smp::IFailure::Fail);
    }
    if (dynamic_cast<Smp::IField *>(object)) {
      interfaces.emplace_back("Smp::IField");
      attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
        const auto *field = dynamic_cast<const Smp::IField *>(obj);
        value["view"] = save(field->GetView());
        value["input"] = field->IsInput();
        value["output"] = field->IsOutput();
        value["state"] = field->IsState();
        value["type"] = save(field->GetType());
      });

      def("IsInput", &Smp::IField::IsInput);
      def("IsOutput", &Smp::IField::IsOutput);
      def("IsState", &Smp::IField::IsState);
      def("GetView", &Smp::IField::GetView);
      def("GetType", &Smp::IField::GetType);

      if (dynamic_cast<Smp::ISimpleField *>(object)) {
        interfaces.emplace_back("Smp::ISimpleField");
        attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
          const auto *simpleField =
              dynamic_cast<const Smp::ISimpleField *>(obj);
          value["value"] = save(simpleField->GetValue());
        });

        def("GetValue", &Smp::ISimpleField::GetValue);
        def("SetValue", &SetFieldValue, arg("value"));

        if (dynamic_cast<Smp::IForcibleField *>(object)) {
          interfaces.emplace_back("Smp::IForcibleField");
          attributes.emplace_back([](::Smp::IObject *obj, wvalue &value) {
            auto *forcibleField = dynamic_cast<Smp::IForcibleField *>(obj);
            value["forced"] = forcibleField->IsForced();
          });
          def("Force", &ForceFieldValue, arg("value"));
          def("Unforce", &Smp::IForcibleField::Unforce);
          def("Freeze", &Smp::IForcibleField::Freeze);
          def("IsForced", &Smp::IForcibleField::IsForced);
        }
      }
      if (dynamic_cast<Smp::IArrayField *>(object)) {
        interfaces.emplace_back("Smp::IArrayField");
        attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
          const auto *arrayField = dynamic_cast<const Smp::IArrayField *>(obj);
          value["size"] = arrayField->GetSize();
        });

        def("GetSize", &Smp::IArrayField::GetSize);
        def("GetItem", &Smp::IArrayField::GetItem, arg("index"));
      }

      if (dynamic_cast<Smp::ISimpleArrayField *>(object)) {
        interfaces.emplace_back("Smp::ISimpleArrayField");
        attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
          const auto *simpleArrayField =
              dynamic_cast<const Smp::ISimpleArrayField *>(obj);
          value["size"] = simpleArrayField->GetSize();
        });

        def("GetSize", &Smp::ISimpleArrayField::GetSize);
        def("GetValue", &Smp::ISimpleArrayField::GetValue, arg("index"));
        def("SetValue", &SetSimpleArrayFieldValue, arg("index"), arg("value"));
      }
      if (dynamic_cast<Smp::IStructureField *>(object)) {
        interfaces.emplace_back("Smp::IStructureField");
        attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
          const auto *structureField =
              dynamic_cast<const Smp::IStructureField *>(obj);
          value["fields"] = save(structureField->GetFields());
        });

        def("GetFields", &Smp::IStructureField::GetFields);
        def("GetField", &Smp::IStructureField::GetField, arg("name"));
      }

      if (const auto *dataflowField =
              dynamic_cast<const Smp::IDataflowField *>(object)) {
        interfaces.emplace_back("Smp::IDataflowField");
        def("Connect", &Smp::IDataflowField::Connect, arg("target"));
        def("Push", &Smp::IDataflowField::Push);
      }
    }

    if (dynamic_cast<Smp::IProperty *>(object)) {
      interfaces.emplace_back("Smp::IProperty");
      attributes.emplace_back([](const ::Smp::IObject *obj, wvalue &value) {
        const auto *property = dynamic_cast<const Smp::IProperty *>(obj);
        value["access"] = save(property->GetAccess());
        value["view"] = save(property->GetView());
        value["type"] = save(property->GetType());
      });
      def("GetAccess", &Smp::IProperty::GetAccess);
      def("GetView", &Smp::IProperty::GetView);
      def("GetValue", &Smp::IProperty::GetValue);
      def("SetValue", &SetPropertyValue, arg("value"));
    }

    if (dynamic_cast<Smp::IOperation *>(object)) {
      interfaces.emplace_back("Smp::IOperation");
      def("GetView", &Smp::IOperation::GetView);
      def("GetReturnParameter", &Smp::IOperation::GetReturnParameter);
      def("GetParameter", &Smp::IOperation::GetParameter, arg("name"));
      def("GetParameters", &Smp::IOperation::GetParameters);

      methods.try_emplace("Invoke", [](::Smp::ISimulator *sim,
                                       ::Smp::IObject *obj,
                                       const crow::query_string &query) {
        auto *operation = dynamic_cast<Smp::IOperation *>(obj);

        auto request = std::unique_ptr<::Smp::IRequest,
                                       std::function<void(::Smp::IRequest *)>>(
            operation->CreateRequest(),
            [operation](::Smp::IRequest *r) { operation->DeleteRequest(r); });
        if (!request)
          Xsmp::Exception::throwException(
              operation, "invalid operation", "Cannot invoke Operation",
              "The operation '", operation->GetName(), "' is not invokable.");

        // copy the query to pop the parameter(s)
        auto q = query;
        // initialize in/inout parameter values
        for (const auto *parameter : *operation->GetParameters()) {
          if (parameter->GetDirection() !=
              Smp::Publication::ParameterDirectionKind::PDK_Out) {

            if (const auto *value = q.pop(parameter->GetName())) {
              request->SetParameterValue(
                  request->GetParameterIndex(parameter->GetName()),
                  load(parameter->GetType()->GetPrimitiveTypeKind(), value));
            } else {
              throwMissingArgument(parameter->GetName());
            }
          }
        }
        // check that no remaining arguments
        checkArgs(q);

        operation->Invoke(request.get());
        crow::json::wvalue value;
        // extract return parameter value if any
        if (const auto *returnParameter = operation->GetReturnParameter()) {
          value[returnParameter->GetName()] = save(request->GetReturnValue());
        }

        // extract inout/out parameters value if any
        for (const auto *parameter : *operation->GetParameters()) {
          if (parameter->GetDirection() !=
              Smp::Publication::ParameterDirectionKind::PDK_In)
            value[parameter->GetName()] = save(request->GetParameterValue(
                request->GetParameterIndex(parameter->GetName())));
        }

        return value;
      });
    }

    attributes.emplace_back(
        [interfaces](const ::Smp::IObject *obj, wvalue &value) {
          value["name"] = obj->GetName();
          value["description"] = obj->GetDescription();
          value["cpp_type"] = Xsmp::Helper::TypeName(obj);
          value["path"] = Xsmp::Helper::GetPath(obj);
          auto i = interfaces;
          value["interfaces"] = std::move(i);
        });
  }
  crow::json::wvalue getInfo(Smp::IObject *object) const {
    crow::json::wvalue value;
    // process all attributes
    for (const auto &attribute : attributes) {
      attribute(object, value);
    }
    return value;
  }
  crow::response invoke(::Smp::ISimulator *sim, Smp::IObject *object,
                        const std::string &method, const crow::request &req) {

    if (auto it = methods.find(method); it != methods.end()) {
      try {
        return it->second(sim, object, req.url_params);
      } catch (const std::exception &e) {
        return crow::response{500, e.what()};
      } catch (...) {
        return crow::response{500, "Unexpected error."};
      }
    }
    return crow::response{404, "Method '" + method + "' not found."};
  }

private:
  template <typename Class, typename Ret, typename... Args,
            typename... ArgsName>
  void def(const char *name, Ret (Class::*method)(Args...) const,
           const ArgsName &...args) {

    static_assert(
        sizeof...(ArgsName) == sizeof...(Args),
        "Number of provided argument names does not match the method's arity.");
    methods.emplace(
        name,
        [method,
         args...](::Smp::ISimulator *sim, ::Smp::IObject *object,
                  const crow::query_string &query) -> crow::json::wvalue {
          auto *obj = dynamic_cast<Class *>(object);
          checkArgs(query, args...);
          if constexpr (std::is_same_v<void, Ret>) {
            (obj->*method)(extractArgAndConvert<Args>(sim, query, args)...);
            return {};
          } else {
            return save((obj->*method)(
                extractArgAndConvert<Args>(sim, query, args)...));
          }
        });
  }
  template <typename Class, typename Ret, typename... Args,
            typename... ArgsName>
  void def(const char *name, Ret (Class::*method)(Args...),
           const ArgsName &...args) {
    static_assert(
        sizeof...(ArgsName) == sizeof...(Args),
        "Number of provided argument names does not match the method's arity.");
    methods.emplace(
        name,
        [method,
         args...](::Smp::ISimulator *sim, ::Smp::IObject *object,
                  const crow::query_string &query) -> crow::json::wvalue {
          auto *obj = dynamic_cast<Class *>(object);
          checkArgs(query, args...);
          if constexpr (std::is_same_v<void, Ret>) {
            (obj->*method)(extractArgAndConvert<Args>(sim, query, args)...);
            return {};
          } else {
            return save((obj->*method)(
                extractArgAndConvert<Args>(sim, query, args)...));
          }
        });
  }
  template <typename Class, typename Ret, typename... Args,
            typename... ArgsName>
  void def(const char *name, Ret (*function)(Class &, Args...),
           const ArgsName &...args) {
    methods.emplace(
        name,
        [function,
         args...](::Smp::ISimulator *sim, ::Smp::IObject *object,
                  const crow::query_string &query) -> crow::json::wvalue {
          static_assert(sizeof...(ArgsName) == sizeof...(Args),
                        "Number of provided argument names does not match the "
                        "method's arity.");
          checkArgs(query, args...);
          if constexpr (std::is_same_v<void, Ret>) {
            function(*dynamic_cast<Class *>(object),
                     extractArgAndConvert<Args>(sim, query, args)...);
            return {};
          } else {
            return save(
                function(*dynamic_cast<Class *>(object),
                         extractArgAndConvert<Args>(sim, query, args)...));
          }
        });
  }

  using method = std::function<crow::json::wvalue(
      ::Smp::ISimulator *, ::Smp::IObject *, const crow::query_string &)>;
  std::map<std::string_view, method> methods;
  using attribute = std::function<void(::Smp::IObject *, crow::json::wvalue &)>;
  std::vector<attribute> attributes;
};

class Server : public crow::SimpleApp {
public:
  Server() {
    // define your endpoint at the root directory
    CROW_ROUTE((*this), "/")([]() { return "Xsmp Server"; });

    CROW_ROUTE((*this), "/createSimulator")
    ([this](const crow::request &req) {
      const auto *libraryName = req.url_params.get("libraryName");
      const auto *factoryName = req.url_params.get("factoryName");
      const auto *name = req.url_params.get("name");
      const auto *description = req.url_params.get("description");

      if (_simulators.find(name) != _simulators.end()) {
        ::Xsmp::Exception::throwDuplicateName(nullptr, name, nullptr);
      }
      void *handle = ::Xsmp::LoadLibrary(libraryName);
      if (!handle) {
        ::Xsmp::Exception::throwLibraryNotFound(nullptr, libraryName,
                                                ::Xsmp::GetLastError());
      }
      auto *factory = ::Xsmp::GetSymbol<::Smp::ISimulator *(*)(::Smp::String8,
                                                               ::Smp::String8)>(
          handle, factoryName);
      if (!factory) {
        ::Xsmp::Exception::throwInvalidLibrary(nullptr, libraryName,
                                               ::Xsmp::GetLastError());
      }
      _simulators.try_emplace(name, std::unique_ptr<::Smp::ISimulator>(
                                        (*factory)(name, description)));

      return crow::response{200};
    });

    CROW_ROUTE((*this), "/sim")
    ([this](const crow::request &) {
      crow::json::wvalue::list list;

      for (const auto &[name, sim] : _simulators) {
        list.emplace_back(name);
      }

      return static_cast<crow::json::wvalue>(list);
    });

    CROW_ROUTE((*this), "/sim/<path>")
    ([this](const crow::request &req, const std::string &path) {
      if (size_t pos = path.find('@'); pos != std::string::npos) {
        auto base = path.substr(0, pos);
        auto [sim, object] = Resolve(base);
        if (object) {
          auto method = path.substr(pos + 1, path.length());
          return crow::response{
              _types.try_emplace(typeid(*object), object)
                  .first->second.invoke(sim, object, method, req)};
        }
      } else {

        auto [sim, object] = Resolve(path);
        if (object) {
          return crow::response{_types.try_emplace(typeid(*object), object)
                                    .first->second.getInfo(object)};
        }
      }

      return crow::response{404, "Could not resolve path '" + path + "'."};
    });
  }

  std::pair<::Smp::ISimulator *, ::Smp::IObject *>
  Resolve(const std::string &path) {
    if (size_t pos = path.find('/'); pos != std::string::npos) {
      auto simName = path.substr(0, pos);

      if (const auto &it = _simulators.find(simName); it != _simulators.end()) {
        auto *sim = it->second.get();
        return std::make_pair(
            sim, Xsmp::Helper::Resolve(
                     sim, path.substr(pos + 1, path.length()).c_str()));
      }
    } else if (const auto &it = _simulators.find(path);
               it != _simulators.end()) {
      auto *sim = it->second.get();
      return std::make_pair(sim, sim);
    }
    return std::make_pair(nullptr, nullptr);
  }

private:
  std::unordered_map<std::type_index, type> _types;
  std::map<std::string, std::unique_ptr<::Smp::ISimulator>, std::less<>>
      _simulators;
};

int main() {
  Server app; // define your crow application

  // set the port, set the app to run on multiple threads, and run the app
  app.port(18080).multithreaded().server_name("XsmpServer").run();
}
