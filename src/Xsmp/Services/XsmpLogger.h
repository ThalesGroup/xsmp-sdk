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

#ifndef XSMP_SERVICES_XSMPLOGGER_H_
#define XSMP_SERVICES_XSMPLOGGER_H_

#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/LogMessageKind.h>
#include <Xsmp/Services/XsmpLoggerGen.h>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

// ----------------------------------------------------------------------------
// ------------------------ Types and Interfaces ------------------------
// ----------------------------------------------------------------------------

namespace Xsmp::Services {

class LoggerProcessor;
/// This class is thread safe: it is possible to QueryLogMessageKind and Log at any time
class XsmpLogger final: public XsmpLoggerGen {
public:
    // ------------------------------------------------------------------------------------
    // -------------------------- Constructors/Destructor --------------------------
    // ------------------------------------------------------------------------------------

    /// Constructor setting name, description and parent.
    /// @param name Name of new model instance.
    /// @param description Description of new model instance.
    /// @param parent Parent of new model instance.
    XsmpLogger(::Smp::String8 name, ::Smp::String8 description,
            ::Smp::IObject *parent, ::Smp::ISimulator *simulator);

    /// Virtual destructor to release memory.
    ~XsmpLogger() noexcept override = default;

    /// Return identifier of log message kind by name.
    ///
    /// @remarks This method can be used for predefined log message
    ///          kinds, but is especially useful for user-defined log
    ///          message kinds.
    ///          It is guaranteed that this method always returns the
    ///          same id for the same messageKindName string.
    /// @param   messageKindName Name of log message kind.
    /// @return  Identifier of log message kind.
    ::Smp::Services::LogMessageKind QueryLogMessageKind(
            ::Smp::String8 messageKindName) override;

    /// This function logs a message to the simulation log.
    /// @param   sender Object that sends the message.
    /// @param   message The message to log.
    /// @param   kind Kind of message.
    void Log(const ::Smp::IObject *sender, ::Smp::String8 message,
            ::Smp::Services::LogMessageKind kind = 0) override;

    void Restore(::Smp::IStorageReader *reader) override;

    void Store(::Smp::IStorageWriter *writer) override;
private:
    friend class ::Xsmp::Component::Helper;
    std::unique_ptr<LoggerProcessor> _processor;
    // init pre-defined kinds: keep ordered
    std::vector<std::string> _logMessageKinds { LMK_InformationName,
            LMK_EventName, LMK_WarningName, LMK_ErrorName, LMK_DebugName };
    std::mutex _mutex { };
};
} // namespace Xsmp::Services

#endif // XSMP_SERVICES_XSMPLOGGER_H_
