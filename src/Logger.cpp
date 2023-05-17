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
#include <strstream>
#include "DateTime.h"

#ifdef MSC_VER 
#define PATH_SEPERATOR "\\"
#else
#define PATH_SEPERATOR "/"
#endif

namespace simple_logger
{
    Log::Log(const std::string& dir, const std::string& fileName) : logDir(dir + "/log/"), logFileName(fileName)
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
            userWriter->Close();
        }

        if (remoteWriter != nullptr) {
            remoteWriter->Close();
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

    void Log::SetColorfulFont(bool enable)
    {
        colorfulFont = enable;
    }

    bool Log::IsColorfulFont() const
    {
        return colorfulFont;
    }

    void Log::AddModule(int module, const std::string& name)
    {
        modulesMap[module] = name;
    }

    void Log::AddModule(const std::unordered_map<int, std::string>& modules)
    {
        std::copy(modules.begin(), modules.end(), std::inserter(modulesMap, modulesMap.end()));
    }

    void Log::RemoveModule(int module)
    {
        modulesMap.erase(module);
    }

    void Log::ClearAllModule()
    {
        modulesMap.clear();
    }

    void Log::AddAndFilter(const std::string& filterString)
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        andFilters.push_back(filterString);
    }

    void Log::AddAndFilter(const std::list<std::string>& filterList)
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        std::copy(filterList.begin(), filterList.end(), std::back_inserter(andFilters));
    }

    void Log::ClearAndFilter(const std::string& filterString)
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        andFilters.erase(std::remove_if(andFilters.begin(), andFilters.end(), [&filterString](const std::string& currentFilterString) { return currentFilterString == filterString; }));
    }

    void Log::ClearAndFilter(const std::list<std::string>& filterList)
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        std::for_each(andFilters.begin(), andFilters.end(), [this](const std::string& filterString) { ClearOrFilter(filterString); });
    }

    void Log::ClearAndFilter()
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        andFilters.clear();
    }

    void Log::AddOrFilter(const std::string& filterString)
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        orFilters.push_back(filterString);
    }

    void Log::AddOrFilter(const std::list<std::string>& filterList)
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        std::copy(filterList.begin(), filterList.end(), std::back_inserter(orFilters));
    }

    void Log::ClearOrFilter(const std::string& filterString)
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        orFilters.erase(std::remove_if(orFilters.begin(), orFilters.end(), [&filterString](const std::string& currentFilterString) { return currentFilterString == filterString; }));
    }

    void Log::ClearOrFilter(const std::list<std::string>& filterList)
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        std::for_each(orFilters.begin(), orFilters.end(), [this](const std::string& filterString) { ClearOrFilter(filterString); });
    }

    void Log::ClearOrFilter()
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        orFilters.clear();
    }

    void Log::AddModuleFilter(int module)
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        moduleFilters.insert(module);
    }

    void Log::AddModuleFilter(const std::list<int>& moduleList)
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        std::copy(moduleList.begin(), moduleList.end(), std::inserter(moduleFilters, moduleFilters.end()));
    }

    void Log::ClearModuleFilter(int module)
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        std::erase_if(moduleFilters, [&module](int currentModule) { return currentModule == module; });
    }

    void Log::ClearModuleFilter(const std::list<int>& moduleList)
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        std::for_each(moduleList.begin(), moduleList.end(), [this](int module) { ClearModuleFilter(module); });
    }

    void Log::ClearModuleFilter()
    {
        std::lock_guard<std::mutex> lock(filterMutex);
        moduleFilters.clear();
    }

    void Log::ClearAllFilter()
    {
        ClearAndFilter();
        ClearOrFilter();
        ClearModuleFilter();
    }

    bool Log::IsLogQueEmpty() const
    {
        return logQue.empty();
    }

    bool Log::NeedFilter(int module)
    {
        if (moduleFilters.empty()) {
            return false;
        }

        std::lock_guard<std::mutex> lock(filterMutex);
        return moduleFilters.find(module) != moduleFilters.end();
    }

    bool Log::NeedFilterWithAndRule(const std::string& msg)
    {
        if (andFilters.empty()) {
            return false;
        }

        bool found = false;
        std::unique_lock<std::mutex> lock(filterMutex);
        for (const std::string& filter : andFilters) {
            if (msg.find(filter) == std::string::npos) {
                lock.unlock();
                found = true;
                break;
            }
        }

        return !found;
    }

    bool Log::NeedFilterWithOrRule(const std::string& msg)
    {
        if (orFilters.empty()) {
            return false;
        }

        bool found = false;
        std::unique_lock<std::mutex> lock(filterMutex);
        for (const std::string& filter : orFilters) {
            if (msg.find(filter) != std::string::npos) {
                lock.unlock();
                found = true;
                break;
            }
        }

        return found;
    }

    bool Log::NeedFilter(int module, const std::string& msg)
    {
        return NeedFilter(module) || NeedFilterWithAndRule(msg) || NeedFilterWithOrRule(msg);
    }

    void Log::Write(LogLevel level, int module, const char* fileName, int line, const char* funcName, std::thread::id threadId, const std::string& msg)
    {
        if (NeedFilter(module, msg)) {
            return;
        }

        std::stringstream ss;
        ss << threadId;
        uint id;
        ss >> id;

        Write(level, WriteMode::Append, module, fileName, line, funcName, id, msg);
    }

    void Log::WriteLine(LogLevel level, int module, const char* fileName, int line, const char* funcName, std::thread::id threadId, const std::string& msg)
    {
        if (NeedFilter(module, msg)) {
            return;
        }

        std::stringstream ss;
        ss << threadId;
        uint id;
        ss >> id;

        Write(level, WriteMode::Newline, module, fileName, line, funcName, id, msg);
    }

    void Log::UpdateCurrentDate(const std::string& currentTime)
    {
        std::string date = currentTime.substr(0, 10);
        if (date != currentDate) {
            currentDate = date;
            dateChanged = true;
        }
    }

    std::string Log::FormatLog(LogLevel level, int module, const std::string& fileName, int line, const std::string& funcName, uint threadId, const std::string& info)
    {
        std::string currentTime = GetLocalDateTimeWithMilliSecond();
        UpdateCurrentDate(currentTime);

        if (!detailMode) {
            return format("{} [{}][{}]: {}", currentTime, LogLevelToStr(level), GetModuleName(module), info);
        }

        return format("{} [{}][{}]{}(line: {}, method: {}, thread: {}): {}",
            currentTime, LogLevelToStr(level), GetModuleName(module), fileName.substr(fileName.find_last_of(PATH_SEPERATOR) + 1), line, funcName, threadId, info);
    }

    void Log::Write(LogLevel level, WriteMode writeMode, int module, const char* fileName, int line, const char* funcName, uint threadId, const std::string& msg)
    {
        if (exit || !IsLogSwitchOn(level)) {
            return;
        }

        std::string str = FormatLog(level, module, fileName, line, funcName, threadId, msg);
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
            WriteToRemoteWriter(logStr);
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

        userWriter->Write(msg);
    }

    void Log::WriteToRemoteWriter(const std::string& msg)
    {
        if (!IsOutputTypeOn(OutputType::RemoteServer) || remoteWriter == nullptr) {
            return;
        }

        remoteWriter->Write(msg);
    }

    void Log::SetUserWriter(std::shared_ptr<UserDefinedWriter>& writer)
    {
        userWriter = writer;
    }

    void Log::SetRemoteWriter(std::shared_ptr<UserDefinedWriter>& writer)
    {
        remoteWriter = writer;
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

    std::string Log::GetModuleName(int module)
    {
        if (auto itr = modulesMap.find(module); itr != modulesMap.end()) {
            return itr->second;
        }

        return "";
    } 
}

