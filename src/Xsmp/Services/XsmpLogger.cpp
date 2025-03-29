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

#include <Smp/IComponent.h>
#include <Smp/IObject.h>
#include <Smp/ISimulator.h>
#include <Smp/IStorageReader.h>
#include <Smp/IStorageWriter.h>
#include <Smp/PrimitiveTypes.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Smp/Services/LogMessageKind.h>
#include <Xsmp/Component.h>
#include <Xsmp/DateTime.h>
#include <Xsmp/Duration.h>
#include <Xsmp/Helper.h>
#include <Xsmp/Persist.h>
#include <Xsmp/Persist/StdString.h>
#include <Xsmp/Persist/StdVector.h>
#include <Xsmp/Services/XsmpLogger.h>
#include <Xsmp/Services/XsmpLoggerGen.h>
#include <algorithm>
#include <cctype>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <queue>
#include <regex>
#include <string>
#include <string_view>
#include <thread>
#include <unordered_set>
#include <utility>
#include <vector>

namespace Xsmp::Services {

namespace {
// trim from start (in place)
inline void ltrim(std::string &str) {
  str.erase(str.begin(),
            std::find_if(str.begin(), str.end(),
                         [](unsigned char chr) { return !std::isspace(chr); }));
}

// trim from end (in place)
inline void rtrim(std::string &str) {
  str.erase(std::find_if(str.rbegin(), str.rend(),
                         [](unsigned char chr) { return !std::isspace(chr); })
                .base(),
            str.end());
}

// trim from both ends (in place)
inline void trim(std::string &str) {
  ltrim(str);
  rtrim(str);
}
inline std::string unescape(std::string_view str) {
  std::string result;
  result.reserve(str.size());
  auto it = str.begin();
  while (it != str.end()) {
    if (*it == '\\') {
      if (++it != str.end()) {
        switch (*it) {
        case '\'':
          result += '\'';
          break;
        case '"':
          result += '\"';
          break;
        case '?':
          result += '\?';
          break;
        case '\\':
          result += '\\';
          break;
        case 'a':
          result += '\a';
          break;
        case 'b':
          result += '\b';
          break;
        case 'f':
          result += '\f';
          break;
        case 'n':
          result += '\n';
          break;
        case 'r':
          result += '\r';
          break;
        case 't':
          result += '\t';
          break;
        case 'v':
          result += '\v';
          break;
        default:
          result += *it;
          break;
        }
        ++it;
      }
      continue;
    }
    result += *it;

    ++it;
  }
  return result;
}

inline std::vector<std::string> split(std::string_view str, char delimiter) {
  std::vector<std::string> result;

  auto end = str.find(delimiter);
  decltype(end) start = 0;
  while (end != std::string::npos) {
    auto token = std::string(str.substr(start, end - start));
    trim(token);
    result.push_back(token);
    start = end + 1;
    end = str.find(delimiter, start);
  }
  auto token = std::string(str.substr(start, str.size() - start));
  trim(token);
  if (!token.empty()) {
    result.push_back(token);
  }
  return result;
}
} // namespace
struct LogEntry {
  std::string sender;
  std::string msg;
  std::string kind;
  ::Xsmp::DateTime zuluTime;
  ::Xsmp::Duration simulationTime;
  ::Xsmp::DateTime epochTime;
  ::Xsmp::Duration missionTime;
};

constexpr const char *_basePath = "log";

class Layout {
public:
  Layout() = default;
  Layout(const Layout &) = delete;
  Layout(Layout &&) = delete;
  Layout &operator=(const Layout &) = delete;
  Layout &operator=(Layout &&) = delete;
  virtual ~Layout() noexcept = default;
  virtual void Append(std::ostream &stream, const LogEntry &entry) const = 0;
};

class SimpleLayout final : public Layout {
public:
  SimpleLayout() = default;
  SimpleLayout(const SimpleLayout &) = delete;
  SimpleLayout(SimpleLayout &&) = delete;
  SimpleLayout &operator=(const SimpleLayout &) = delete;
  SimpleLayout &operator=(SimpleLayout &&) = delete;
  ~SimpleLayout() noexcept override = default;
  void Append(std::ostream &stream, const LogEntry &entry) const override {
    stream << entry.simulationTime << "\t" << entry.sender << '\t' << entry.kind
           << '\t' << entry.msg << '\n';
  }
};

class PatternLayout final : public Layout {
public:
  PatternLayout(
      const std::string &path,
      const std::map<std::string, std::string, std::less<>> &properties)
      : _pattern{ComputePatternLayout(path, properties)} {}
  ~PatternLayout() noexcept override = default;
  PatternLayout(const PatternLayout &) = delete;
  PatternLayout(PatternLayout &&) = delete;
  PatternLayout &operator=(const PatternLayout &) = delete;
  PatternLayout &operator=(PatternLayout &&) = delete;
  void Append(std::ostream &stream, const LogEntry &entry) const override {
    using namespace std::chrono;

    auto format = [&stream](std::string::const_iterator &it,
                            const auto &value) {
      if (auto fmt = GetFormatting(it); fmt.empty()) {
        stream << static_cast<::Smp::Int64>(value);
      } else {
        value.to_stream(stream, fmt);
      }
    };

    auto it = _pattern.begin();
    while (it != _pattern.end()) {
      if (*it == '%') {
        ++it;
        switch (*it) {
        case 'p':
          stream << entry.sender;
          break;
        case 'k':
          stream << entry.kind;
          break;
        case 'm':
          stream << entry.msg;
          break;
        case 'd':
          format(it, entry.zuluTime);
          break;
        case 'S':
          format(it, entry.simulationTime);
          break;
        case 'E':
          format(it, entry.epochTime);
          break;
        case 'M':
          format(it, entry.missionTime);
          break;
        case 't':
          stream << '\t';
          break;
        case 'n':
          stream << '\n';
          break;
        case '%':
          stream << '%';
          break;
        default:
          stream << "%" << *it;
          break;
        }
      }
      // write all other chars
      else {
        stream << *it;
      }
      ++it;
    }
    stream.flush();
  }

private:
  std::string _pattern;

  static std::string
  ComputePatternLayout(const std::string &path,
                       const std::map<std::string, std::string, std::less<>>
                           &properties) noexcept {

    if (auto pattern = properties.find(path + ".conversionPattern");
        pattern != properties.end()) {
      /*
       *  %p Output the Path of the sender IObject
       *  %k Output the message kind name
       *  %m Output the message
       *  %d Output the Date of the msg
       * %S Output the Simulation Time
       * %E Output the epoch time
       * %M Output the mission time
       * %n Output std::endl
       * %t Output \t
       * %% Output a single %
       */
      return unescape(pattern->second);
    }
    // use default pattern
    return "%d{%F %T}%t%S%t%p%t%k%t%m%n";
  }
  static std::string GetFormatting(std::string::const_iterator &it) noexcept {
    std::string formatting;
    if (it[1] == '{') {
      it += 2;
      while (*it != '}') {
        formatting.push_back(*it);
        ++it;
      }
    }
    return formatting;
  }
};
class Appender {
public:
  Appender(const std::string &path,
           const std::map<std::string, std::string, std::less<>> &properties) {
    auto key = path + ".layout";
    if (auto layout = properties.find(key); layout != properties.end()) {

      if (layout->second == "SimpleLayout") {
        _layout = std::make_unique<SimpleLayout>();
      } else if (layout->second == "PatternLayout") {
        _layout = std::make_unique<PatternLayout>(key, properties);
      } else {
        std::cerr << "Layout " << layout->second
                  << " does not exist: only SimpleLayout and PatternLayout are "
                     "supported."
                  << '\n';
      }
    }
    // create a PatternLayout by default
    else {
      _layout = std::make_unique<PatternLayout>(key, properties);
    }
    if (auto levels = properties.find(path + ".levels");
        levels != properties.end()) {
      for (const auto &value : split(levels->second, ',')) {
        _levels.emplace(value);
      }
    }

    if (auto pathRegex = properties.find(path + ".path");
        pathRegex != properties.end()) {
      try {
        _pathRegex = std::regex{unescape(pathRegex->second),
                                std::regex::ECMAScript | std::regex::optimize};
      } catch (const std::regex_error &e) {
        std::cerr << "Invalid regex expression: \"" << pathRegex->second
                  << "\" ->" << e.what() << '\n';
      }
    }
  }
  virtual ~Appender() noexcept = default;
  Appender(const Appender &) = delete;
  Appender &operator=(const Appender &) = delete;
  Appender(Appender &&) = delete;
  Appender &operator=(Appender &&) = delete;

  void Append(const LogEntry &entry) {

    if ((_levels.empty() || _levels.find(entry.kind) != _levels.end()) &&
        (!_pathRegex || std::regex_match(entry.sender, *_pathRegex))) {
      DoAppend(entry);
    }
  }

protected:
  virtual void DoAppend(const LogEntry &entry) = 0;
  void Append(std::ostream &stream, const LogEntry &entry) const {
    _layout->Append(stream, entry);
  }

private:
  std::unique_ptr<Layout> _layout;
  std::unordered_set<std::string> _levels;
  std::optional<std::regex> _pathRegex;
};

class ConsoleAppender final : public Appender {
  // mutex to protect from simultaneous access to std::cout
  static std::mutex _mutex;

public:
  using Appender::Appender;
  ~ConsoleAppender() noexcept override = default;
  ConsoleAppender(const ConsoleAppender &) = delete;
  ConsoleAppender &operator=(const ConsoleAppender &) = delete;
  ConsoleAppender(ConsoleAppender &&) = delete;
  ConsoleAppender &operator=(ConsoleAppender &&) = delete;
  void DoAppend(const LogEntry &entry) override {
    const std::scoped_lock lck{_mutex};
    Append(std::cout, entry);
  }
};
std::mutex ConsoleAppender::_mutex;

class FileAppender final : public Appender {

public:
  FileAppender(
      const std::string &path,
      const std::map<std::string, std::string, std::less<>> &properties)
      : Appender(path, properties), _stream{GetFileName(path, properties)} {}
  ~FileAppender() noexcept override = default;
  FileAppender(const FileAppender &) = delete;
  FileAppender &operator=(const FileAppender &) = delete;
  FileAppender(FileAppender &&) = delete;
  FileAppender &operator=(FileAppender &&) = delete;
  void DoAppend(const LogEntry &entry) override { Append(_stream, entry); }

private:
  static std::string GetFileName(
      const std::string &path,
      const std::map<std::string, std::string, std::less<>> &properties) {
    if (auto file = properties.find(path + ".File"); file != properties.end()) {
      return file->second;
    }
    return "simulator.log";
  }
  std::ofstream _stream;
};
class LoggerProcessor {
public:
  LoggerProcessor() {
    auto properties = parseProperties();

    if (auto rootLogger =
            properties.find(std::string(_basePath) + ".rootLogger");
        rootLogger != properties.end()) {
      for (const auto &appender : split(rootLogger->second, ',')) {
        CreateAppender(appender, properties);
      }
    }
    // create a ConsoleAppender by default
    else {
      _appenders.emplace_back(std::make_unique<ConsoleAppender>(
          std::string(_basePath) + ".appender.default", properties));
    }

    // initialize the working thread
    workingThread = std::thread{&LoggerProcessor::Process, this};
  }
  LoggerProcessor(const LoggerProcessor &) = delete;
  LoggerProcessor &operator=(const LoggerProcessor &) = delete;
  LoggerProcessor(LoggerProcessor &&) = delete;
  LoggerProcessor &operator=(LoggerProcessor &&) = delete;
  ~LoggerProcessor() {
    // terminate the working thread
    Stop();
    _cv.notify_one();
    if (workingThread.joinable()) {
      workingThread.join();
    }
  }
  void Log(const ::Smp::IObject *sender, ::Smp::String8 msg,
           const std::string &kind, ::Smp::DateTime zuluTime,
           ::Smp::Duration simulationTime, ::Smp::DateTime epochTime,
           ::Smp::Duration missionTime) {
    Push(sender, msg, kind, zuluTime, simulationTime, epochTime, missionTime);
    _cv.notify_one();
  }

private:
  void Stop() {
    const std::scoped_lock lck(_mutex);
    running = false;
  }
  void Push(const ::Smp::IObject *sender, ::Smp::String8 msg,
            const std::string &kind, ::Smp::DateTime zuluTime,
            ::Smp::Duration simulationTime, ::Smp::DateTime epochTime,
            ::Smp::Duration missionTime) {
    const std::scoped_lock lck(_mutex);
    _logs.push({::Xsmp::Helper::GetPath(sender), msg, kind,
                Xsmp::DateTime{zuluTime}, Xsmp::Duration{simulationTime},
                Xsmp::DateTime{epochTime}, Xsmp::Duration{missionTime}});
  }
  void Process() {
    std::unique_lock lck(_mutex);
    while (running) {
      while (!_logs.empty()) {

        const auto &log = _logs.front();
        lck.unlock();

        for (auto const &appender : _appenders) {
          appender->Append(log);
        }
        lck.lock();
        _logs.pop();
      }
      _cv.wait(lck, [this] { return !running || !_logs.empty(); });
    }

    // process remaining logs if any
    while (!_logs.empty()) {
      const auto &log = _logs.front();
      for (auto const &appender : _appenders) {
        appender->Append(log);
      }
      _logs.pop();
    }
  }
  [[nodiscard]] static std::map<std::string, std::string, std::less<>>
  parseProperties() {
    std::map<std::string, std::string, std::less<>> properties;

    if (std::ifstream pFile("XsmpLogger.properties"); pFile.is_open()) {

      for (std::string line; std::getline(pFile, line);) {
        trim(line);

        if (line.empty() || line[0] == '#') {
          continue;
        }
        auto delimiterPos = line.find('=');
        if (delimiterPos != std::string::npos) {
          auto name = line.substr(0, delimiterPos);
          auto value = line.substr(delimiterPos + 1);
          // trim spaces arround "="
          rtrim(name);
          ltrim(value);
          properties.try_emplace(name, value);
        } else {
          std::cerr << "Invalid line: " << line << '\n';
        }
      }
    }

    return properties;
  }

  void CreateAppender(
      const std::string &name,
      const std::map<std::string, std::string, std::less<>> &properties) {
    auto path = std::string(_basePath) + ".appender." + name;
    // find the associated class
    auto appender = properties.find(path);
    if (appender != properties.end()) {
      if (appender->second == "ConsoleAppender") {
        _appenders.emplace_back(
            std::make_unique<ConsoleAppender>(path, properties));
      } else if (appender->second == "FileAppender") {
        _appenders.emplace_back(
            std::make_unique<FileAppender>(path, properties));
      } else {
        std::cerr << "Appender " << appender->second
                  << " does not exist: only ConsoleAppender and FileAppender "
                     "are supported."
                  << '\n';
      }

    } else {
      std::cerr
          << "Could not create appender '" << name << "'. Add entry '" << path
          << "=ConsoleAppender' to create a Console appender or add entry '"
          << path << "=FileAppender' to create a File appender." << '\n';
    }
  }
  std::mutex _mutex;
  std::condition_variable _cv;
  std::queue<LogEntry> _logs;
  std::vector<std::unique_ptr<Appender>> _appenders;

  bool running{true};
  std::thread workingThread;
};

XsmpLogger::XsmpLogger(::Smp::String8 name, ::Smp::String8 description,
                       ::Smp::IComposite *parent, ::Smp::ISimulator *simulator)
    : XsmpLoggerGen::XsmpLoggerGen(name, description, parent, simulator),
      _processor{std::make_unique<LoggerProcessor>()} {}

::Smp::Services::LogMessageKind
XsmpLogger::QueryLogMessageKind(::Smp::String8 messageKindName) {
  const std::scoped_lock lck{_mutex};
  // this implementation is not very efficient.
  // we do not expect to call this method too often and to have many different
  // kinds of msg

  auto msgKindAccess = _logMessageKinds.read();
  if (auto it = std::find(msgKindAccess.get().begin(),
                          msgKindAccess.get().end(), messageKindName);
      it != msgKindAccess.get().end()) {
    return static_cast<::Smp::Services::LogMessageKind>(
        it - msgKindAccess.get().begin());
  }
  // release read access
  msgKindAccess.unlock();
  // create write access
  auto msgKindAccessWrite = _logMessageKinds.write();
  // the kind is a direct cast from UInt32 to ::Smp::Services::LogMessageKind
  auto kind = static_cast<::Smp::Services::LogMessageKind>(
      msgKindAccessWrite.get().size());
  msgKindAccessWrite.get().emplace_back(messageKindName);
  return kind;
}

void XsmpLogger::Log(const ::Smp::IObject *sender, ::Smp::String8 message,
                     ::Smp::Services::LogMessageKind kind) {
  const std::scoped_lock lck{_mutex};

  // the index is a direct cast from ::Smp::Services::LogMessageKind to
  // ::Smp::UInt32
  auto index = static_cast<::Smp::UInt32>(kind);
  auto msgKindAccess = _logMessageKinds.read();
  const auto &msgKind = index < msgKindAccess.get().size()
                            ? msgKindAccess.get()[index]
                            : "<unknown: " + std::to_string(kind) + ">";
  msgKindAccess.unlock();

  if (GetSimulator() && GetSimulator()->GetTimeKeeper()) {
    auto const *tk = GetSimulator()->GetTimeKeeper();
    _processor->Log(sender, message, msgKind, tk->GetZuluTime(),
                    tk->GetSimulationTime(), tk->GetEpochTime(),
                    tk->GetMissionTime());
  } else {
    _processor->Log(sender, message, msgKind,
                    static_cast<::Smp::DateTime>(::Xsmp::DateTime::now()), 0, 0,
                    0);
  }
}

void XsmpLogger::Restore(::Smp::IStorageReader *reader) {
  ::Xsmp::Persist::Restore(GetSimulator(), this, reader,
                           _logMessageKinds.write().get());
}

void XsmpLogger::Store(::Smp::IStorageWriter *writer) {
  ::Xsmp::Persist::Store(GetSimulator(), this, writer,
                         _logMessageKinds.read().get());
}

} // namespace Xsmp::Services
