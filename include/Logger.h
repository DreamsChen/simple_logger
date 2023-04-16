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
#include <queue>
#include "Formatter.h"


#define DBG_TRACE(log, fmt, ...) log.WriteLine(LogLevel::Debug, __FILE__, __LINE__, __FUNCTION__, FORMAT(fmt, ##__VA_ARGS__))
#define DBG_INFO(log, fmt, ...) log.WriteLine(LogLevel::Info, __FILE__, __LINE__, __FUNCTION__, FORMAT(fmt, ##__VA_ARGS__))
#define DBG_WARN(log, fmt, ...) log.WriteLine(LogLevel::Warn, __FILE__, __LINE__, __FUNCTION__, FORMAT(fmt, ##__VA_ARGS__))
#define DBG_ERROR(log, fmt, ...) log.WriteLine(LogLevel::Error, __FILE__, __LINE__, __FUNCTION__, FORMAT(fmt, ##__VA_ARGS__))
#define DBG_FATAL(log, fmt, ...) log.WriteLine(LogLevel::Fatal, __FILE__, __LINE__, __FUNCTION__, FORMAT(fmt, ##__VA_ARGS__))

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
        UserDefined = 4,
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
        Log(const std::string& dir, const std::string& fileName, uint outputFlag = static_cast<uint>(OutputType::Console), bool detailMode = false);
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
        void SetUserWriter(UserDefinedWriter* userWriter);
        bool IsLogQueEmpty() const;
        
        void Write(LogLevel level, const char* fileName, int line, const char* funcName, const std::string& msg);
        void WriteLine(LogLevel level, const char* fileName, int line, const char* funcName, const std::string& msg);

    private:
        std::string FormatLog(LogLevel level, const std::string& fileName, int line, const std::string& funcName, const std::string& info);
        void Write(LogLevel level, WriteMode writeMode, const char* fileName, int line, const char* funcName, const std::string& msg);
        void WriteToConsole(const std::string& msg);
        void WriteToLogFile(const std::string& msg);
        void WriteToUserWriter(const std::string& msg); 
        const char* LogLevelToStr(LogLevel level);
        void WritingWorker();
        void UpdateCurrentDate(const std::string& currentTime);

    private:
        uint outputFlag;
        int logFlag = (uint)LogLevel::Info | (uint)LogLevel::Warn | (uint)LogLevel::Error | (uint)LogLevel::Fatal;
        bool detailMode;
        bool exit = false;
        bool dateChanged = false;

        std::string logDir;
        std::string logFileName;
        std::string currentDate;
        std::queue<std::string> logQue;
        std::thread writerThread;
        std::mutex writeMutex;
        std::mutex queMutex;

        std::ofstream writer;
        UserDefinedWriter* userWriter = nullptr;
    };
};

#endif // !LOGGER_H
