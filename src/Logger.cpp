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

#include "Logger.h"

#include <filesystem>
#include <iostream>
#include "DateTime.h"

namespace simple_logger
{
    Log::Log(const std::string& dir, const std::string& fileName, uint outputFlag, bool detailMode) : outputFlag(outputFlag), detailMode(detailMode), logDir(dir + "/log/"), logFileName(fileName)
    {
        currentDate = GetLocalDate();
        std::string filePath = logDir + currentDate + "_" + fileName;

        if (!std::filesystem::exists(logDir)) {
            std::filesystem::create_directory(std::filesystem::path(logDir));
        }

        writer.open(filePath, std::ios::out | std::ios::app);

        writerThread = std::thread(&Log::WritingWorker, this);
    }

    Log::~Log()
    {
        exit = true;

        if (writerThread.joinable()) {
            writerThread.join();
        }

        writer.close();
        if (userWriter != nullptr) {
            delete userWriter;
        }
    }

    uint Log::GetOutputFlag() const
    {
        return outputFlag;
    }

    bool Log::IsOutputTypeOn(OutputType type) const
    {
        return (outputFlag & ((uint)type)) != 0;
    }

    void Log::SetOutputTypeOn(OutputType outputType)
    {
        outputFlag |= (uint)outputType;
    }

    void Log::SetOutputTypeOff(OutputType outputType)
    {
        outputFlag &= ~(uint)outputType;
    }

    void Log::DisableLog()
    {
        outputFlag = 0;
    }

    bool Log::IsLogSwitchOn(LogLevel level) const
    {
        return (logFlag & (uint)level) != 0;
    }

    void Log::SetLogSwitchOn(LogLevel level)
    {
        logFlag |= (uint)level;
    }

    void Log::SetLogSwitchOff(LogLevel level)
    {
        logFlag &= ~(uint)level;
    }

    void Log::SetDetailMode(bool enable)
    {
        detailMode = enable;
    }

    bool Log::GetDetailMode() const
    {
        return detailMode;
    }

    bool Log::IsLogQueEmpty() const
    {
        return logQue.empty();
    }
    

    void Log::Write(LogLevel level, const char* fileName, int line, const char* funcName, const std::string& msg)
    {
        Write(level, WriteMode::Append, fileName, line, funcName, msg);
    }

    void Log::WriteLine(LogLevel level, const char* fileName, int line, const char* funcName, const std::string& msg)
    {
        Write(level, WriteMode::Newline, fileName, line, funcName, msg);
    }

    void Log::UpdateCurrentDate(const std::string& currentTime)
    {
        std::string date = currentTime.substr(0, 10);
        if (date != currentDate) {
            currentDate = date;
            dateChanged = true;
        }
    }

    std::string Log::FormatLog(LogLevel level, const std::string& fileName, int line, const std::string& funcName, const std::string& info)
    {
        std::string currentTime = GetLocalDateTimeWithMilliSecond();
        UpdateCurrentDate(currentTime);

        if (!detailMode) {
            return format("{} [{}]: {}", currentTime, LogLevelToStr(level), info);
        }

        return format("{} [{}]{}(line: {}, method: {}): {}",
            currentTime, LogLevelToStr(level), fileName.substr(fileName.find_last_of("\\") + 1), line, funcName, info);
    }

    void Log::Write(LogLevel level, WriteMode writeMode, const char* fileName, int line, const char* funcName, const std::string& msg)
    {
        if (exit || !IsLogSwitchOn(level)) {
            return;
        }

        std::string str = FormatLog(level, fileName, line, funcName, msg);
        if (writeMode == WriteMode::Newline) {
            str.append("\r\n");
        }

        std::unique_lock<std::mutex> lock(queMutex);
        logQue.emplace(str);

        return;
    }

    void Log::WritingWorker()
    {
        while (!exit) {
            std::unique_lock<std::mutex> lock(queMutex);
            if (logQue.empty()) {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                continue;
            }

            std::string logStr = logQue.front();
            logQue.pop();
            lock.unlock();

            // Only one writing thread, no need to lock for the below action.
            WriteToConsole(logStr);
            WriteToLogFile(logStr);
            WriteToUserWriter(logStr);
        }
    }

    void Log::WriteToConsole(const std::string& msg)
    {
        if (!IsOutputTypeOn(OutputType::Console)) {
            return;
        }

        std::unique_lock<std::mutex> lock(queMutex);
        std::cout << msg;
    }

    void Log::WriteToLogFile(const std::string& msg)
    {
        if (!IsOutputTypeOn(OutputType::LogFile)) {
            return;
        }

        if (dateChanged) {
            writer.close();
            writer.open(logDir + currentDate + "_" + logFileName, std::ios::out | std::ios::app);
            dateChanged = false;
        }

        writer << msg;
        writer.flush();
    }

    void Log::WriteToUserWriter(const std::string& msg)
    {
        if (!IsOutputTypeOn(OutputType::UserDefined) || userWriter == nullptr) {
            return;
        }

        std::unique_lock<std::mutex> lock(queMutex);
        userWriter->Write(msg);
    }

    void Log::SetUserWriter(UserDefinedWriter* writer)
    {
        userWriter = writer;
    }

    const char* Log::LogLevelToStr(LogLevel level)
    {
        switch (level) {
        case LogLevel::Debug:
            return "Debug";
        case LogLevel::Info:
            return "Info";
        case LogLevel::Warn:
            return "Warn";
        case LogLevel::Error:
            return "Error";
        case LogLevel::Fatal:
            return "Fatal";
        }

        return "Unknow";
    }
}

