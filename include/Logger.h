// Copyright(c) 2023-present, Dreams Chen.

// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//    http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#ifndef LOGGER_H
#define LOGGER_H

#include <thread>
#include <memory>
#include <string_view>
#include <unordered_map>
#include <unordered_set>
#include "Formatter.h"

// used with module level print micro to simplify coding, for example:
// #define EXAMPLE_DEBUG(fmt, ...) DBG_DEBUG(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
// EXAMPLE_DEBUG("This is a print example. str={}", "test");
// see ../example/Example.cpp for more detail.
#define DBG_DEBUG(log, mod, fmt, ...) log.Write(simple_logger::LogLevel::Debug, mod, __FILE__, __LINE__, __FUNCTION__, std::this_thread::get_id(), FORMAT(fmt, ##__VA_ARGS__))
#define DBG_INFO(log, mod, fmt, ...) log.Write(simple_logger::LogLevel::Info, mod, __FILE__, __LINE__, __FUNCTION__, std::this_thread::get_id(), FORMAT(fmt, ##__VA_ARGS__))
#define DBG_WARN(log, mod, fmt, ...) log.Write(simple_logger::LogLevel::Warn, mod, __FILE__, __LINE__, __FUNCTION__, std::this_thread::get_id(), FORMAT(fmt, ##__VA_ARGS__))
#define DBG_ERROR(log, mod, fmt, ...) log.Write(simple_logger::LogLevel::Error, mod, __FILE__, __LINE__, __FUNCTION__, std::this_thread::get_id(), FORMAT(fmt, ##__VA_ARGS__))
#define DBG_FATAL(log, mod, fmt, ...) log.Write(simple_logger::LogLevel::Fatal, mod, __FILE__, __LINE__, __FUNCTION__, std::this_thread::get_id(), FORMAT(fmt, ##__VA_ARGS__))

// micro definition for time performent evaluation 
#define START_TIME() simple_logger::Now _begin = simple_logger::GetCurrentTime();
#define END_TIME() simple_logger::Now _end = simple_logger::GetCurrentTime();
#define USED_TIME(_msg) \
    auto _duration = std::chrono::duration_cast<std::chrono::microseconds>(_end - _begin);  \
    std::cout << _msg << ": " << _duration.count() << "us, or " << _duration.count() / 1000 << "ms, or " << _duration.count() / 1000000 << "s" << std::endl;

namespace simple_logger
{
    enum class LogLevel
    {
        Debug = 1,
        Info = 2,
        Warn = 4,
        Error = 8,
        Fatal = 16,
    };

    enum class OutputType
    {
        None = 0,
        Console = 1,
        LogFile = 2,
        RemoteServer = 4,
        UserDefined = 8,
    };

    enum class WriteMode
    {
        Append = 0,
        Newline,
    };

    class UserDefinedWriter
    {
    public:
        virtual ~UserDefinedWriter() = default;
    public:
        virtual void Write(const std::string& str) = 0;
        virtual void Close() {};
    };

    template <class ...Args>
    uint32_t MakeFlag(Args... args)
    {
        return (... | static_cast<int>(args));
    }

    class Log
    {
    public:
        Log(const char* dir, const char* fileName, uint32_t outputFlag = MakeFlag(OutputType::LogFile), uint32_t logLevelFlag = MakeFlag(LogLevel::Info, LogLevel::Warn, LogLevel::Error, LogLevel::Fatal), bool detailMode = true);
        Log(std::string_view dir, std::string_view fileName, uint32_t outputFlag = MakeFlag(OutputType::LogFile), uint32_t logLevelFlag = MakeFlag(LogLevel::Info, LogLevel::Warn, LogLevel::Error, LogLevel::Fatal), bool detailMode = true);

        Log(const Log& log) = delete;
        Log(const Log&& log) = delete;
        Log& operator=(const Log& log) = delete;
        Log& operator=(const Log&& log) = delete;

        ~Log();

    public:
        uint32_t GetOutputFlag() const;
        bool IsOutputTypeOn(OutputType type) const;
        void SetOutputTypeOn(OutputType outputType);
        void SetOutputTypeOff(OutputType outputType);
        void DisableLog();
        bool IsLogSwitchOn(LogLevel level) const;
        void SetLogSwitchOn(LogLevel level);
        void SetLogSwitchOff(LogLevel level);
        void SetDetailMode(bool enable);
        bool IsDetailMode() const;
        void SetColorfulFont(bool enable);
        bool IsColorfulFont() const;
        void SetReverseFilter(bool enable);
        bool IsReverseFilter() const;

        void AddModule(int module, const std::string& name);
        void AddModule(const std::unordered_map<int, std::string>& modules);
        void RemoveModule(int module);
        void ClearAllModule();

        void AddAndFilter(const std::string& filterString);
        void AddAndFilter(const std::unordered_set<std::string>& filterList);
        void ClearAndFilter(const std::string& filterString);
        void ClearAndFilter(const std::unordered_set<std::string>& filterList);
        void ClearAndFilter();
        
        void AddOrFilter(const std::string& filterString);
        void AddOrFilter(const std::unordered_set<std::string>& filterList);
        void ClearOrFilter(const std::string& filterString);
        void ClearOrFilter(const std::unordered_set<std::string>& filterList);
        void ClearOrFilter();

        void AddModuleFilter(int module);
        void AddModuleFilter(const std::unordered_set<int>& moduleList);
        void ClearModuleFilter(int module);
        void ClearModuleFilter(const std::unordered_set<int>& moduleList);
        void ClearModuleFilter();

        void ClearAllFilter();

        void SetUserWriter(std::shared_ptr<UserDefinedWriter> m_userWriter);
        void SetRemoteWriter(std::shared_ptr<UserDefinedWriter> m_remoteWriter);
        bool IsLogQueEmpty() const;
        
        void Write(LogLevel level, int module, const char* fileName, int line, const char* funcName, std::thread::id threadId, const std::string& msg, WriteMode writeMode = WriteMode::Newline);

        // Close function should be called munually before the program exit.
        void Close();

    private:
        class LogImpl;
        std::unique_ptr<LogImpl> m_impl;
    };
};

#endif // !LOGGER_H
