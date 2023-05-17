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

#if defined(_MSC_VER) && (_MSC_VER >= 1200)
# pragma once
#endif // defined(_MSC_VER) && (_MSC_VER >= 1200)

#include <fstream>
#include <thread>
#include <mutex>
#include <memory>
#include <queue>
#include <unordered_map>
#include <unordered_set>
#include "Formatter.h"


#define DBG_TRACE(log, fmt, ...) log.WriteLine(LogLevel::Debug, 0, __FILE__, __LINE__, __FUNCTION__, std::this_thread::get_id(), FORMAT(fmt, ##__VA_ARGS__))
#define DBG_INFO(log, fmt, ...) log.WriteLine(LogLevel::Info, 0, __FILE__, __LINE__, __FUNCTION__, std::this_thread::get_id(), FORMAT(fmt, ##__VA_ARGS__))
#define DBG_WARN(log, fmt, ...) log.WriteLine(LogLevel::Warn, 0, __FILE__, __LINE__, __FUNCTION__, std::this_thread::get_id(), FORMAT(fmt, ##__VA_ARGS__))
#define DBG_ERROR(log, fmt, ...) log.WriteLine(LogLevel::Error, 0, __FILE__, __LINE__, __FUNCTION__, std::this_thread::get_id(), FORMAT(fmt, ##__VA_ARGS__))
#define DBG_FATAL(log, fmt, ...) log.WriteLine(LogLevel::Fatal, 0, __FILE__, __LINE__, __FUNCTION__, std::this_thread::get_id(), FORMAT(fmt, ##__VA_ARGS__))

// micro definition for time performent evaluation 
#define START_TIME() Now _begin = GetCurrentTime();
#define END_TIME() Now _end = GetCurrentTime();
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

    using uint = unsigned int;

    class UserDefinedWriter
    {
    public:
        virtual ~UserDefinedWriter() = default;
    public:
        virtual void Write(const char* str) = 0;
        void Write(const std::string& str)
        {
            Write(str.c_str());
        }

        virtual void Close() {};
    };

    class Log
    {
    public:
        Log(const std::string& dir, const std::string& fileName);
        Log(const Log& log) = delete;
        Log(const Log&& log) = delete;
        Log& operator=(const Log& log) = delete;
        Log& operator=(const Log&& log) = delete;

        virtual ~Log();

    public:
        uint GetOutputFlag() const;
        bool IsOutputTypeOn(OutputType type) const;
        void SetOutputTypeOn(OutputType outputType);
        void SetOutputTypeOff(OutputType outputType);
        void DisableLog();
        bool IsLogSwitchOn(LogLevel level) const;
        void SetLogSwitchOn(LogLevel level);
        void SetLogSwitchOff(LogLevel level);
        void SetDetailMode(bool enable);
        bool GetDetailMode() const;
        void SetColorfulFont(bool enable);
        bool IsColorfulFont() const;

        void AddModule(int module, const std::string& name);
        void AddModule(const std::unordered_map<int, std::string>& modules);
        void RemoveModule(int module);
        void ClearAllModule();

        void AddAndFilter(const std::string& filterString);
        void AddAndFilter(const std::list<std::string>& filterList);
        void ClearAndFilter(const std::string& filterString);
        void ClearAndFilter(const std::list<std::string>& filterList);
        void ClearAndFilter();
        
        void AddOrFilter(const std::string& filterString);
        void AddOrFilter(const std::list<std::string>& filterList);
        void ClearOrFilter(const std::string& filterString);
        void ClearOrFilter(const std::list<std::string>& filterList);
        void ClearOrFilter();

        void AddModuleFilter(int module);
        void AddModuleFilter(const std::list<int>& moduleList);
        void ClearModuleFilter(int module);
        void ClearModuleFilter(const std::list<int>& moduleList);
        void ClearModuleFilter();

        void ClearAllFilter();

        void SetUserWriter(std::shared_ptr<UserDefinedWriter>& userWriter);
        void SetRemoteWriter(std::shared_ptr<UserDefinedWriter>& remoteWriter);
        bool IsLogQueEmpty() const;
        
        void Write(LogLevel level, int module, const char* fileName, int line, const char* funcName, std::thread::id threadId, const std::string& msg);
        void WriteLine(LogLevel level, int module, const char* fileName, int line, const char* funcName, std::thread::id threadId, const std::string& msg);

    private:
        std::string FormatLog(LogLevel level, int module, const std::string& fileName, int line, const std::string& funcName, uint threadId, const std::string& info);
        void Write(LogLevel level, WriteMode writeMode, int module, const char* fileName, int line, const char* funcName, uint threadId, const std::string& msg);

        bool NeedFilter(int module, const std::string& msg);
        bool NeedFilter(int module);
        bool NeedFilterWithAndRule(const std::string& msg);
        bool NeedFilterWithOrRule(const std::string& msg);

        void WriteToConsole(const std::string& msg);
        void WriteToLogFile(const std::string& msg);
        void WriteToUserWriter(const std::string& msg); 
        void WriteToRemoteWriter(const std::string& msg);
        const char* LogLevelToStr(LogLevel level);
        void WritingWorker();
        void UpdateCurrentDate(const std::string& currentTime);

        std::string GetModuleName(int module);

    private:
        uint outputFlag = static_cast<uint>(OutputType::Console);
        int logFlag = (uint)LogLevel::Info | (uint)LogLevel::Warn | (uint)LogLevel::Error | (uint)LogLevel::Fatal;
        bool detailMode = true;
        bool exit = false;
        bool dateChanged = false;
        bool colorfulFont = true;          // only use in console terminal.
        
        std::string logDir;
        std::string logFileName;
        std::string currentDate;
        std::queue<std::string> logQue;
        std::thread writerThread;
        std::mutex writeMutex;
        std::mutex queMutex;
        std::mutex filterMutex;
        std::unordered_map<int, std::string> modulesMap;
        std::list<std::string> andFilters;
        std::list<std::string> orFilters;
        std::unordered_set<int> moduleFilters;

        std::ofstream writer;
        std::shared_ptr<UserDefinedWriter> userWriter = nullptr;
        std::shared_ptr<UserDefinedWriter> remoteWriter = nullptr;
    };
};

#endif // !LOGGER_H
