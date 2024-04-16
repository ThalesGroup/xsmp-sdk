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

#ifndef PYTHON_SMP_SERVICES_ILOGGER_H_
#define PYTHON_SMP_SERVICES_ILOGGER_H_

#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/ILogger.h>
#include <python/ecss_smp.h>

inline void RegisterILogger(const py::module_ &m) {
  py::class_<::Smp::Services::ILogger, ::Smp::IService>(
      m, "ILogger", py::multiple_inheritance())

      .def("QueryLogMessageKind",
           &::Smp::Services::ILogger::QueryLogMessageKind,
           py::arg("message_kind_name"),
           "Return identifier of log message kind by name.")

      .def("Log", &::Smp::Services::ILogger::Log, py::arg("sender"),
           py::arg("message"), py::arg("kind") = 0,
           "This function logs a message to the simulation log.")

      .def(
          "LogDebug",
          [](::Smp::Services::ILogger &self, const ::Smp::IObject *sender,
             ::Smp::String8 message) {
            self.Log(sender, message, ::Smp::Services::ILogger::LMK_Debug);
          },
          py::arg("sender"), py::arg("message"),
          "This function logs a debug message to the simulation log.")

      .def(
          "LogEvent",
          [](::Smp::Services::ILogger &self, const ::Smp::IObject *sender,
             ::Smp::String8 message) {
            self.Log(sender, message, ::Smp::Services::ILogger::LMK_Event);
          },
          py::arg("sender"), py::arg("message"),
          "This function logs an event message to the simulation log.")

      .def(
          "LogInfo",
          [](::Smp::Services::ILogger &self, const ::Smp::IObject *sender,
             ::Smp::String8 message) {
            self.Log(sender, message,
                     ::Smp::Services::ILogger::LMK_Information);
          },
          py::arg("sender"), py::arg("message"),
          "This function logs an information message to the simulation log.")

      .def(
          "LogWarning",
          [](::Smp::Services::ILogger &self, const ::Smp::IObject *sender,
             ::Smp::String8 message) {
            self.Log(sender, message, ::Smp::Services::ILogger::LMK_Warning);
          },
          py::arg("sender"), py::arg("message"),
          "This function logs a warning message to the simulation log.")

      .def(
          "LogError",
          [](::Smp::Services::ILogger &self, const ::Smp::IObject *sender,
             ::Smp::String8 message) {
            self.Log(sender, message, ::Smp::Services::ILogger::LMK_Error);
          },
          py::arg("sender"), py::arg("message"),
          "This function logs an error message to the simulation log.")

      .def_readonly_static("LMK_Information",
                           &::Smp::Services::ILogger::LMK_Information,
                           "The message contains general information.")

      .def_readonly_static("LMK_Event", &::Smp::Services::ILogger::LMK_Event,
                           "The message has been sent from an event, typically "
                           "from a state transition.")

      .def_readonly_static("LMK_Warning",
                           &::Smp::Services::ILogger::LMK_Warning,
                           "The message contains a warning.")

      .def_readonly_static("LMK_Error", &::Smp::Services::ILogger::LMK_Error,
                           "The message has been raised because of an error.")

      .def_readonly_static("LMK_Debug", &::Smp::Services::ILogger::LMK_Debug,
                           "The message contains debug information.")

      .def_readonly_static("LMK_InformationName",
                           &::Smp::Services::ILogger::LMK_InformationName,
                           "The message contains general information.")

      .def_readonly_static("LMK_EventName",
                           &::Smp::Services::ILogger::LMK_EventName,
                           "The message has been sent from an event, typically "
                           "from a state transition.")

      .def_readonly_static("LMK_WarningName",
                           &::Smp::Services::ILogger::LMK_WarningName,
                           "The message contains a warning.")

      .def_readonly_static("LMK_ErrorName",
                           &::Smp::Services::ILogger::LMK_ErrorName,
                           "The message has been raised because of an error.")

      .def_readonly_static("LMK_DebugName",
                           &::Smp::Services::ILogger::LMK_DebugName,
                           "The message contains debug information.")

      .doc() =
      R"(This interface gives access to the Logger Service.
All objects in a simulation can log messages using this service. Objects can either use pre-defined log message kinds, or define their own message kinds.)";
}

#endif // PYTHON_SMP_SERVICES_ILOGGER_H_
