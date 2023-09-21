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

#include <Smp/Exception.h>
#include <Smp/IAggregate.h>
#include <Smp/IArrayField.h>
#include <Smp/ICollection.h>
#include <Smp/IContainer.h>
#include <Smp/IDynamicInvocation.h>
#include <Smp/IEntryPoint.h>
#include <Smp/IEntryPointPublisher.h>
#include <Smp/IEventConsumer.h>
#include <Smp/IEventProvider.h>
#include <Smp/IEventSink.h>
#include <Smp/IEventSource.h>
#include <Smp/IFailure.h>
#include <Smp/IFallibleModel.h>
#include <Smp/IField.h>
#include <Smp/IOperation.h>
#include <Smp/IProperty.h>
#include <Smp/IReference.h>
#include <Smp/ISimpleArrayField.h>
#include <Smp/ISimpleField.h>
#include <Smp/ISimulator.h>
#include <Smp/IStructureField.h>
#include <Smp/Services/ILogger.h>
#include <Xsmp/Helper.h>
#include <algorithm>
#include <cstring>
#include <memory>

#if defined(__GNUG__)
#    include <cxxabi.h>
#endif

namespace Xsmp::Helper {

void SafeExecute(::Smp::ISimulator *simulator,
        const ::Smp::IEntryPoint *entryPoint) {

    if (simulator) {
        try {
            simulator->GetLogger()->Log(entryPoint, "Execute()",
                    ::Smp::Services::ILogger::LMK_Debug);

            entryPoint->Execute();

            simulator->GetLogger()->Log(entryPoint, "return from Execute()",
                    ::Smp::Services::ILogger::LMK_Debug);
        }
        // SMP Exception
        catch (const ::Smp::Exception &e) {
            auto msg = std::string("Exception ") + e.GetName()
                    + " thrown during execution.";
            simulator->GetLogger()->Log(entryPoint, msg.c_str(),
                    ::Smp::Services::ILogger::LMK_Error);
            simulator->GetLogger()->Log(e.GetSender(), e.what(),
                    ::Smp::Services::ILogger::LMK_Error);
            simulator->Abort();
        }
        // standard c++ exception
        catch (const std::exception &e) {

            auto msg = std::string("Exception thrown during execution: ")
                    + e.what();
            simulator->GetLogger()->Log(entryPoint, msg.c_str(),
                    ::Smp::Services::ILogger::LMK_Error);
            simulator->Abort();
        }
        // other exceptions
        catch (...) {
            simulator->GetLogger()->Log(entryPoint,
                    "Unexpected exception thrown during execution.",
                    ::Smp::Services::ILogger::LMK_Error);
            simulator->Abort();
        }
    }
    else
        entryPoint->Execute();
}
namespace {
std::string ComputePath(const ::Smp::IObject *obj) {

    if (!obj)
        return "<null>";

    if (dynamic_cast<const ::Smp::ISimulator*>(obj))
        return "/";

    auto *parent = obj->GetParent();

    if (dynamic_cast<::Smp::ISimulator*>(parent))
        return ComputePath(parent) + obj->GetName();

    // use '/' separator between components
    if (dynamic_cast<const ::Smp::IComponent*>(obj))
        return ComputePath(parent) + "/" + obj->GetName();

    // use '.' separator between all others elements : fields, entry points, event sink/sources, ...
    return ComputePath(parent) + "." + obj->GetName();
}

std::string GetNextSegment(::Smp::String8 *path, ::Smp::Char8 *separator) {
    *separator = '\0';
    if (!(*path) || *path[0] == '\0')
        return "";

    //skip starting '/', '.', './'
    while (true) {
        while (**path == '/') {
            ++(*path);
            *separator = '/';
        }
        if ((*path)[0] == '.' && (*path)[1] != '.') {
            ++(*path);
            *separator = '.';
            continue;
        }
        break;
    }
    auto *it = *path;
    if (*it == '[') {
        ++it;
    }
    while (true) {
        switch (*it) {
        case '.':
            while (*(++it) == '.')
                ;
            [[fallthrough]];
        case '/':
        case '[':
        case '\0': {
            std::string result { *path, it };
            *path = it;
            return result;
        }
        default:
            break;
        }
        ++it;
    }
}
} // namespace

std::string GetPath(const ::Smp::IObject *obj) {
    return ComputePath(obj);
}

::Smp::IObject* Resolve(const ::Smp::FieldCollection *fields,
        ::Smp::String8 path) {
    ::Smp::Char8 separator;
    auto segment = GetNextSegment(&path, &separator);
    if (segment.empty())
        return nullptr;

    if (segment == "..") {
        if (auto const *container = fields->GetParent())
            return Resolve(container->GetParent(), path);
        else
            return nullptr;
    }

    if (auto *field = fields->at(segment.c_str()))
        return Resolve(field, path);
    return nullptr;

}
::Smp::IObject* Resolve(::Smp::IField *field, ::Smp::String8 path) {

    ::Smp::Char8 separator;
    auto segment = GetNextSegment(&path, &separator);
    if (segment.empty())
        return field;

    if (segment == "..")
        return Resolve(field->GetParent(), path);

    if (auto const *structureField =
            dynamic_cast<::Smp::IStructureField*>(field)) {
        if (auto *nestedField = structureField->GetField(segment.c_str()))
            return Resolve(nestedField, path);
    }
    if (auto const *arrayField = dynamic_cast<::Smp::IArrayField*>(field)) {
        if (segment.front() == '[' && segment.back() == ']') {

            try {
                auto size = arrayField->GetSize();
                size_t pos;
                auto value = std::stoll(&segment[1], &pos);
                if (pos + 2 != segment.length())
                    return nullptr;
                // handle negative index (similar to python arrays)
                auto index = static_cast<::Smp::UInt64>(
                        value < 0 ?
                                static_cast<::Smp::Int64>(size) + value : value);

                if (index < size)
                    return Resolve(arrayField->GetItem(index), path);
            }
            catch (std::exception&) {
                //ignore stoll conversion errors
                return nullptr;
            }
        }
    }
    return nullptr;
}
static inline ::Smp::IObject* Resolve(const ::Smp::IComposite *composite,
        ::Smp::String8 name, ::Smp::String8 path) {
    if (auto *containers = composite->GetContainers())
        for (auto const *ctn : *containers)
            if (auto *cmp = ctn->GetComponent(name))
                return Resolve(cmp, path);
    return nullptr;
}
static inline ::Smp::IObject* Resolve(const ::Smp::IAggregate *aggregate,
        ::Smp::String8 name, ::Smp::String8 path) {
    if (auto *references = aggregate->GetReferences())
        for (auto const *ref : *references)
            if (auto *cmp = ref->GetComponent(name))
                return Resolve(cmp, path);
    return nullptr;
}
static inline ::Smp::IObject* Resolve(
        const ::Smp::IDynamicInvocation *dynamicInvocation, ::Smp::String8 name,
        ::Smp::String8 path) {
    if (auto *operations = dynamicInvocation->GetOperations())
        if (auto *operation = operations->at(name))
            return Resolve(operation, path);
    if (auto *properties = dynamicInvocation->GetProperties())
        if (auto *property = properties->at(name))
            return Resolve(property, path);
    return nullptr;
}
static inline ::Smp::IObject* Resolve(::Smp::IObject *object,
        ::Smp::String8 name, ::Smp::String8 path) {

    if (auto const *eventConsumer =
            dynamic_cast<const ::Smp::IEventConsumer*>(object)) {

        if (auto *eventSink = eventConsumer->GetEventSink(name))
            return Resolve(eventSink, path);
    }
    if (auto const *eventProvider =
            dynamic_cast<const ::Smp::IEventProvider*>(object)) {

        if (auto *eventSource = eventProvider->GetEventSource(name))
            return Resolve(eventSource, path);
    }
    if (auto const *entryPointPublisher =
            dynamic_cast<const ::Smp::IEntryPointPublisher*>(object)) {

        if (auto *entryPoint = entryPointPublisher->GetEntryPoint(name))
            return Resolve(entryPoint, path);
    }

    if (auto const *dynamicInvocation =
            dynamic_cast<const ::Smp::IDynamicInvocation*>(object)) {
        if (auto *result = Resolve(dynamicInvocation, name, path))
            return result;
    }

    if (auto const *fallibleModel =
            dynamic_cast<const ::Smp::IFallibleModel*>(object)) {
        if (auto *failure = fallibleModel->GetFailures()->at(name))
            return Resolve(failure, path);
    }
    if (auto const *component = dynamic_cast<const ::Smp::IComponent*>(object)) {

        if (auto *fields = component->GetFields())
            if (auto *field = fields->at(name))
                return Resolve(field, path);
    }

    if (auto *field = dynamic_cast<::Smp::IField*>(object))
        return Resolve(field, path);
    return nullptr;
}

static inline ::Smp::IObject* ResolveComponent(const ::Smp::IObject *object,
        ::Smp::String8 name, ::Smp::String8 path) {
    if (auto const *composite = dynamic_cast<const ::Smp::IComposite*>(object)) {
        if (auto *result = Resolve(composite, name, path))
            return result;
    }

    // check reference last as references are not unique
    if (auto const *aggregate = dynamic_cast<const ::Smp::IAggregate*>(object)) {
        if (auto *result = Resolve(aggregate, name, path))
            return result;
    }
    return nullptr;
}

::Smp::IObject* Resolve(::Smp::IObject *object, ::Smp::String8 path) {

    if (path[0] == '\0' || !object)
        return object;

    ::Smp::Char8 separator;
    auto segment = GetNextSegment(&path, &separator);

    if (segment.empty())
        return object;

    if (segment == "..")
        return Resolve(object->GetParent(), path);

    // by default Containers and references are not accessible
    // This implementation access them if they are prefixed with "_"
    if (segment.front() == '_') {
        if (auto const *composite =
                dynamic_cast<const ::Smp::IComposite*>(object)) {
            if (auto *ctn = composite->GetContainer(segment.c_str() + 1))
                return Resolve(ctn, path);
        }
        if (auto const *aggregate =
                dynamic_cast<const ::Smp::IAggregate*>(object)) {
            if (auto *ref = aggregate->GetReference(segment.c_str() + 1))
                return Resolve(ref, path);
        }
    }

    if (auto *result = Resolve(object, segment.c_str(), path))
        return result;

    if (separator == '/' || separator == '\0')
        return ResolveComponent(object, segment.c_str(), path);

    return nullptr;
}

inline void erase_all(std::string& string, const std::string& search) {
    for (size_t pos = 0;;) {
        pos = string.find(search, pos);
        if (pos == std::string::npos) {
            break;
        }
        string.erase(pos, search.length());
    }
}

std::string demangle(::Smp::String8 name) {
#if defined(__GNUG__)
    int status = 0;
    std::unique_ptr<char, void (*)(void*)> res { abi::__cxa_demangle(name,
            nullptr, nullptr, &status), [](auto *elem) {
        std::free(elem);
    } };
    if (status == 0) {
        return res.get();
    }
    return name;
#else
    std::string result {name};
    erase_all(result, "class ");
    erase_all(result, "struct ");
    erase_all(result, "enum ");
    return result;
#endif
}
std::string TypeName(const ::Smp::IObject *type) {
    return demangle(typeid(*type).name());
}

void CopyString(::Smp::Char8 *destination, size_t size,
        const ::Smp::AnySimple &value) {
    auto *str = static_cast<::Smp::String8>(value);
    if (str) {
        size_t length = std::min(size, std::strlen(str) + 1);
        std::memcpy(destination, str, length);
        destination[size - 1] = '\0';
    }
    else {
        destination[0] = '\0';
    }
}

static inline bool AreEquivalent(const ::Smp::ISimpleArrayField *source,
        const ::Smp::ISimpleArrayField *target) {

    // the type is possibly null in case of an anonymous simple array
    // the Smp::IsimpleArray interface should provide a Smp::PrimitiveTypeKind GetItemKind() const; method
    // && simpleArrayTarget->GetType()->GetPrimitiveTypeKind() == simpleArraySource->GetType()->GetPrimitiveTypeKind()
    if (target->GetSize() == source->GetSize()) {
        for (size_t i = 0; i < source->GetSize(); ++i) {
            if (source->GetValue(i).GetType() != target->GetValue(i).GetType())
                return false;
        }
        return true;
    }
    return false;
}
static inline bool AreEquivalent(const ::Smp::IArrayField *source,
        const ::Smp::IArrayField *target) {
    if (target->GetSize() == source->GetSize()) {
        for (size_t i = 0; i < source->GetSize(); ++i) {
            if (!AreEquivalent(source->GetItem(i), target->GetItem(i)))
                return false;
        }
        return true;
    }
    return false;
}
static inline bool AreEquivalent(const ::Smp::IStructureField *source,
        const ::Smp::IStructureField *target) {
    if (target->GetFields()->size() == source->GetFields()->size()) {
        size_t index = 0;
        for (auto const *f : *source->GetFields()) {
            if (!AreEquivalent(f, target->GetFields()->at(index)))
                return false;
            ++index;
        }
        return true;
    }
    return false;
}
bool AreEquivalent(const ::Smp::IField *source, const ::Smp::IField *target) {

    // check a simple field
    if (auto const *simpleSource =
            dynamic_cast<const ::Smp::ISimpleField*>(source)) {
        auto const *simpleTarget =
                dynamic_cast<const ::Smp::ISimpleField*>(target);
        if (simpleTarget
                && (simpleTarget->GetPrimitiveTypeKind()
                        == simpleSource->GetPrimitiveTypeKind()))
            return true;
    }
    // check a simple array field
    else if (auto const *simpleArraySource =
            dynamic_cast<const ::Smp::ISimpleArrayField*>(source)) {
        auto const *simpleArrayTarget =
                dynamic_cast<const ::Smp::ISimpleArrayField*>(target);
        if (simpleArrayTarget
                && AreEquivalent(simpleArraySource, simpleArrayTarget))
            return true;
    }
    // check an array field
    else if (auto const *arraySource =
            dynamic_cast<const ::Smp::IArrayField*>(source)) {
        auto const *arrayTarget =
                dynamic_cast<const ::Smp::IArrayField*>(target);
        if (arrayTarget && AreEquivalent(arraySource, arrayTarget))
            return true;
    }
    // check a structure field
    else if (auto const *structSource =
            dynamic_cast<const ::Smp::IStructureField*>(source)) {
        auto const *structTarget =
                dynamic_cast<const ::Smp::IStructureField*>(target);
        if (structTarget && AreEquivalent(structSource, structTarget))
            return true;
    }
    else {
        //ignore
    }
    return false;
}
} // namespace Xsmp::Helper
