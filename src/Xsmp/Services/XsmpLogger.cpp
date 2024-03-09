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

#include <Smp/ISimulator.h>
#include <Smp/Services/ITimeKeeper.h>
#include <Xsmp/Component.h>
#include <Xsmp/DateTime.h>
#include <Xsmp/Duration.h>
#include <Xsmp/Persist.h>
#include <Xsmp/Persist/StdVector.h>
#include <Xsmp/Persist/StdString.h>
#include <Xsmp/Services/XsmpLogger.h>
#include <Xsmp/Helper.h>
#include <algorithm>
#include <cctype>
#include <condition_variable>
#include <fstream>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <optional>
#include <queue>
#include <regex>
#include <string_view>
#include <thread>
#include <unordered_set>

namespace Xsmp::Services {

namespace {
// trim from start (in place)
inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

// trim from end (in place)
inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

// trim from both ends (in place)
inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}
inline std::string unescape(std::string_view s) {
    std::string result;
    result.reserve(s.size());
    auto it = s.begin();
    while (it != s.end()) {
        if (*it == '\\') {
            if (++it != s.end())
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
                    break;
                }
            break;
        }
        else {
            result += *it;
        }
        ++it;
    }
    return result;

}

inline std::vector<std::string> split(std::string_view s, char delimiter) {
    std::vector<std::string> result;

    auto end = s.find(delimiter);
    decltype(end) start = 0;
    while (end != std::string::npos) {
        auto token = std::string(s.substr(start, end - start));
        trim(token);
        result.push_back(token);
        start = end + 1;
        end = s.find(delimiter, start);
    }
    auto token = std::string(s.substr(start, s.size() - start));
    trim(token);
    if (!token.empty())
        result.push_back(token);

    return result;
}
}
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
    virtual ~Layout() noexcept = default;
    virtual void Append(std::ostream &stream, const LogEntry &entry) const = 0;
};

class SimpleLayout final: public Layout {
public:
    ~SimpleLayout() noexcept override = default;
    void Append(std::ostream &stream, const LogEntry &entry) const override {
        stream << entry.simulationTime << "\t" << entry.sender << '\t'
                << entry.kind << '\t' << entry.msg << std::endl;
    }
};

class PatternLayout final: public Layout {
public:
    PatternLayout(const std::string &path,
            const std::map<std::string, std::string, std::less<>> &properties) :
            _pattern { ComputePatternLayout(path, properties) } {
    }
    ~PatternLayout() noexcept override = default;

    void Append(std::ostream &stream, const LogEntry &entry) const override {
        using namespace std::chrono;

        auto format = [&stream, this](std::string::const_iterator &it,
                const auto &value) {
            if (auto fmt = GetFormatting(it); fmt.empty())
                stream << static_cast<::Smp::Int64>(value);
            else
                value.to_stream(stream, fmt);

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
            //write all other chars
            else {
                stream << *it;
            }
            ++it;
        }
        stream.flush();
    }
private:
    const std::string _pattern;

    std::string ComputePatternLayout(const std::string &path,
            const std::map<std::string, std::string, std::less<>> &properties) const noexcept {

        if (auto pattern = properties.find(path + ".conversionPattern"); pattern
                != properties.end()) {
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
        //use default pattern
        return "%d{%F %T}%t%S%t%p%t%k%t%m%n";
    }
    std::string GetFormatting(std::string::const_iterator &it) const noexcept {
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

            if (layout->second == "SimpleLayout")
                _layout = std::make_unique<SimpleLayout>();
            else if (layout->second == "PatternLayout")
                _layout = std::make_unique<PatternLayout>(key, properties);
            else
                std::cerr << "Layout " << layout->second
                        << " does not exist: only SimpleLayout and PatternLayout are supported."
                        << std::endl;
        }
        // create a PatternLayout by default
        else
            _layout = std::make_unique<PatternLayout>(key, properties);

        if (auto levels = properties.find(path + ".levels"); levels
                != properties.end()) {
            for (const auto &value : split(levels->second, ','))
                _levels.emplace(value);
        }

        if (auto pathRegex = properties.find(path + ".path"); pathRegex
                != properties.end())
            try {
                _pathRegex = std::regex { unescape(pathRegex->second),
                        std::regex::ECMAScript | std::regex::optimize };
            }
            catch (const std::regex_error &e) {
                std::cerr << "Invalid regex expression: \"" << pathRegex->second
                        << "\" ->" << e.what() << std::endl;
            }

    }
    virtual ~Appender() noexcept = default;
    Appender(const Appender&) = delete;
    Appender& operator =(const Appender&) = delete;

    void Append(const LogEntry &entry) {

        if ((_levels.empty() || _levels.find(entry.kind) != _levels.end())
                && (!_pathRegex || std::regex_match(entry.sender, *_pathRegex)))
            DoAppend(entry);
    }
protected:
    virtual void DoAppend(const LogEntry &entry) = 0;
    inline void Append(std::ostream &stream, const LogEntry &entry) const {
        _layout->Append(stream, entry);
    }
private:
    std::unique_ptr<Layout> _layout { };
    std::unordered_set<std::string> _levels { };
    std::optional<std::regex> _pathRegex { };
};

class ConsoleAppender final : public Appender {
    // mutex to protect from simultaneous access to std::cout
    static std::mutex _mutex;
public:
    using Appender::Appender;
    ~ConsoleAppender() noexcept override = default;
    void DoAppend(const LogEntry &entry) override {
        std::scoped_lock lck { _mutex };
        Append(std::cout, entry);
    }

};
std::mutex ConsoleAppender::_mutex { };

class FileAppender final: public Appender {

public:
    FileAppender(const std::string &path,
            const std::map<std::string, std::string, std::less<>> &properties) :
            Appender(path, properties), _stream { GetFileName(path, properties) } {
    }
    ~FileAppender() noexcept override = default;
    void DoAppend(const LogEntry &entry) override {
        Append(_stream, entry);
    }
private:
    std::string GetFileName(const std::string &path,
            const std::map<std::string, std::string, std::less<>> &properties) const {
        auto key = path + ".File";
        auto file = properties.find(key);
        if (file != properties.end())
            return file->second;
        else
            return "simulator.log";
    }
    std::ofstream _stream;
};
class LoggerProcessor {
public:
    LoggerProcessor() :
            // initialize the working thread
            workingThread { &LoggerProcessor::Process, this } {

        auto properties = parseProperties();

        auto rootLogger = properties.find(
                std::string(_basePath) + ".rootLogger");
        if (rootLogger != properties.end()) {
            for (const auto &appender : split(rootLogger->second, ','))
                CreateAppender(appender, properties);
        }
        // create a ConsoleAppender by default
        else
            _appenders.emplace_back(
                    std::make_unique<ConsoleAppender>(
                            std::string(_basePath) + ".appender.default",
                            properties));

    }

    ~LoggerProcessor() {
        //terminate the working thread
        running = false;
        _cv.notify_all();
        if (workingThread.joinable())
            workingThread.join();
    }
    void Log(const ::Smp::IObject *sender, ::Smp::String8 msg,
            const std::string &kind, ::Smp::DateTime zuluTime,
            ::Smp::Duration simulationTime, ::Smp::DateTime epochTime,
            ::Smp::Duration missionTime) {

        std::unique_lock lck(_mutex);
        _logs.push( { ::Xsmp::Helper::GetPath(sender), msg, kind,
                Xsmp::DateTime { zuluTime }, Xsmp::Duration { simulationTime },
                Xsmp::DateTime { epochTime }, Xsmp::Duration { missionTime } });
        lck.unlock();
        _cv.notify_one();
    }
private:

    void Process() {
        std::unique_lock lck(_mutex);
        while (running) {
            while (!_logs.empty()) {

                const auto &log = _logs.front();
                lck.unlock();

                for (auto const &appender : _appenders)
                    appender->Append(log);

                lck.lock();
                _logs.pop();
            }
            _cv.wait(lck, [this] {
                return !running || !_logs.empty();
            });
        }
    }

    std::map<std::string, std::string, std::less<>> parseProperties() const {
        std::map<std::string, std::string, std::less<>> properties;

        if (std::ifstream pFile("XsmpLogger.properties"); pFile.is_open()) {

            for (std::string line; std::getline(pFile, line);) {
                trim(line);

                if (line.empty() || line[0] == '#')
                    continue;
                auto delimiterPos = line.find("=");
                if (delimiterPos != std::string::npos) {
                    auto name = line.substr(0, delimiterPos);
                    auto value = line.substr(delimiterPos + 1);
                    // trim spaces arround "="
                    rtrim(name);
                    ltrim(value);
                    properties.try_emplace(name, value);
                }
                else
                    std::cerr << "Invalid line: " << line << std::endl;
            }

        }

        return properties;
    }

    void CreateAppender(const std::string &name,
            const std::map<std::string, std::string, std::less<>> &properties) {
        auto path = std::string(_basePath) + ".appender." + name;
        // find the associated class
        auto appender = properties.find(path);
        if (appender != properties.end()) {
            if (appender->second == "ConsoleAppender")
                _appenders.emplace_back(
                        std::make_unique<ConsoleAppender>(path, properties));
            else if (appender->second == "FileAppender")
                _appenders.emplace_back(
                        std::make_unique<FileAppender>(path, properties));
            else
                std::cerr << "Appender " << appender->second
                        << " does not exist: only ConsoleAppender and FileAppender are supported."
                        << std::endl;

        }
        else
            std::cerr << "Could not create appender '" << name
                    << "'. Add entry '" << path
                    << "=ConsoleAppender' to create a Console appender or add entry '"
                    << path << "=FileAppender' to create a File appender."
                    << std::endl;
    }
    std::mutex _mutex { };
    std::condition_variable _cv { };
    std::queue<LogEntry> _logs { };
    std::vector<std::unique_ptr<Appender>> _appenders { };

    bool running { true };
    std::thread workingThread;

};

XsmpLogger::XsmpLogger(::Smp::String8 name, ::Smp::String8 description,
        ::Smp::IComposite *parent, ::Smp::ISimulator *simulator) :
        XsmpLoggerGen::XsmpLoggerGen(name, description, parent, simulator), _processor {
                std::make_unique<LoggerProcessor>() } {

}

::Smp::Services::LogMessageKind XsmpLogger::QueryLogMessageKind(
        ::Smp::String8 messageKindName) {
    std::scoped_lock lck { _mutex };
    // this implementation is not very performant.
    // we do not expect to call this method too often and to have many different kinds of msg

    if (auto it = std::find(_logMessageKinds.begin(), _logMessageKinds.end(),
            messageKindName); it != _logMessageKinds.end())
        return static_cast<::Smp::Services::LogMessageKind>(it
                - _logMessageKinds.begin());

    // the kind is a direct cast from UInt32 to ::Smp::Services::LogMessageKind
    auto kind =
            static_cast<::Smp::Services::LogMessageKind>(_logMessageKinds.size());
    _logMessageKinds.emplace_back(messageKindName);
    return kind;

}

void XsmpLogger::Log(const ::Smp::IObject *sender, ::Smp::String8 message,
        ::Smp::Services::LogMessageKind kind) {
    std::unique_lock lck { _mutex };

    // the index is a direct cast from ::Smp::Services::LogMessageKind to ::Smp::UInt32
    auto index = static_cast<::Smp::UInt32>(kind);
    const auto &msgKind = (
            index < _logMessageKinds.size() ?
                    _logMessageKinds[index] :
                    "<unknown: " + std::to_string(kind) + ">");
    lck.unlock();

    if (GetSimulator() && GetSimulator()->GetTimeKeeper()) {
        auto const *tk = GetSimulator()->GetTimeKeeper();
        _processor->Log(sender, message, msgKind, tk->GetZuluTime(),
                tk->GetSimulationTime(), tk->GetEpochTime(),
                tk->GetMissionTime());
    }
    else
        _processor->Log(sender, message, msgKind,
                static_cast<::Smp::DateTime>(::Xsmp::DateTime::now()), 0, 0, 0);
}

void XsmpLogger::Restore(::Smp::IStorageReader *reader) {
    ::Xsmp::Persist::Restore(GetSimulator(), this, reader, _logMessageKinds);
}

void XsmpLogger::Store(::Smp::IStorageWriter *writer) {
    ::Xsmp::Persist::Store(GetSimulator(), this, writer, _logMessageKinds);
}

} // namespace Xsmp::Services
