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
#include <sstream>
#include "DateTime.h"

#ifdef _MSC_VER 
#define PATH_SEPERATOR "\\"
#else
#define PATH_SEPERATOR "/"
#endif

namespace simple_logger
{
    const char* FONT_STYLE_RED = "\033[31m";
    const char* FONT_STYLE_GREEN = "\033[32m";
    const char* FONT_STYLE_YELLOW = "\033[33m";
    const char* FONT_STYLE_PURPLE = "\033[35m";
    const char* FONT_STYLE_CYAN = "\033[36m";
    const char* FONT_STYLE_CLEAR = "\033[0m";

    Log::Log(const std::string& dir, const std::string& fileName) : m_logDir(dir + "/log/"), m_logFileName(fileName)
    {
        m_currentDate = GetLocalDate();
        std::string filePath = m_logDir + m_currentDate + "_" + fileName;

        if (!std::filesystem::exists(m_logDir)) {
            std::filesystem::create_directory(std::filesystem::path(m_logDir));
        }

        m_fileWriter.open(filePath, std::ios::out | std::ios::app);

        m_writerThread = std::thread(&Log::WritingWorker, this);
    }

    Log::~Log()
    {
        Close();
    }

    uint Log::GetOutputFlag() const
    {
        return m_outputFlag;
    }

    bool Log::IsOutputTypeOn(OutputType type) const
    {
        return (m_outputFlag & ((uint)type)) != 0;
    }

    void Log::SetOutputTypeOn(OutputType outputType)
    {
        m_outputFlag |= (uint)outputType;
    }

    void Log::SetOutputTypeOff(OutputType outputType)
    {
        m_outputFlag &= ~(uint)outputType;
    }

    void Log::DisableLog()
    {
        m_outputFlag = 0;
    }

    bool Log::IsLogSwitchOn(LogLevel level) const
    {
        return (m_logFlag & (uint)level) != 0;
    }

    void Log::SetLogSwitchOn(LogLevel level)
    {
        m_logFlag |= (uint)level;
    }

    void Log::SetLogSwitchOff(LogLevel level)
    {
        m_logFlag &= ~(uint)level;
    }

    void Log::SetDetailMode(bool enable)
    {
        m_detailMode = enable;
    }

    bool Log::IsDetailMode() const
    {
        return m_detailMode;
    }

    void Log::SetColorfulFont(bool enable)
    {
        m_colorfulFont = enable;
    }

    bool Log::IsColorfulFont() const
    {
        return m_colorfulFont;
    }

    void Log::SetReverseFilter(bool enable)
    {
        m_reverseFilter = enable;
    }

    bool Log::IsReverseFilter() const
    {
        return m_reverseFilter;
    }

    void Log::AddModule(int module, const std::string& name)
    {
        std::lock_guard<std::mutex> lock(m_moduleMutex);
        m_modulesMap[module] = name;
    }

    void Log::AddModule(const std::unordered_map<int, std::string>& modules)
    {
        std::lock_guard<std::mutex> lock(m_moduleMutex);
        std::copy(modules.begin(), modules.end(), std::inserter(m_modulesMap, m_modulesMap.end()));
    }

    void Log::RemoveModule(int module)
    {
        std::lock_guard<std::mutex> lock(m_moduleMutex);
        m_modulesMap.erase(module);
    }

    void Log::ClearAllModule()
    {
        std::lock_guard<std::mutex> lock(m_moduleMutex);
        m_modulesMap.clear();
    }

    void Log::AddAndFilter(const std::string& filterString)
    {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        m_andFilters.insert(filterString);
    }

    void Log::AddAndFilter(const std::unordered_set<std::string>& filterList)
    {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        std::copy(filterList.begin(), filterList.end(), std::inserter(m_andFilters, m_andFilters.end()));
    }

    void Log::ClearAndFilter(const std::string& filterString)
    {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        m_andFilters.erase(filterString);
    }

    void Log::ClearAndFilter(const std::unordered_set<std::string>& filterList)
    {
        std::for_each(filterList.begin(), filterList.end(), [this](const std::string& filterString) { ClearAndFilter(filterString); });
    }

    void Log::ClearAndFilter()
    {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        m_andFilters.clear();
    }

    void Log::AddOrFilter(const std::string& filterString)
    {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        m_orFilters.insert(filterString);
    }

    void Log::AddOrFilter(const std::unordered_set<std::string>& filterList)
    {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        std::copy(filterList.begin(), filterList.end(), std::inserter(m_orFilters, m_orFilters.end()));
    }

    void Log::ClearOrFilter(const std::string& filterString)
    {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        m_orFilters.erase(filterString);
    }

    void Log::ClearOrFilter(const std::unordered_set<std::string>& filterList)
    {
        std::for_each(filterList.begin(), filterList.end(), [this](const std::string& filterString) { ClearOrFilter(filterString); });
    }

    void Log::ClearOrFilter()
    {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        m_orFilters.clear();
    }

    void Log::AddModuleFilter(int module)
    {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        m_moduleFilters.insert(module);
    }

    void Log::AddModuleFilter(const std::unordered_set<int>& moduleList)
    {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        std::copy(moduleList.begin(), moduleList.end(), std::inserter(m_moduleFilters, m_moduleFilters.end()));
    }

    void Log::ClearModuleFilter(int module)
    {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        m_moduleFilters.erase(module);
    }

    void Log::ClearModuleFilter(const std::unordered_set<int>& moduleList)
    {
        std::for_each(moduleList.begin(), moduleList.end(), [this](int module) { ClearModuleFilter(module); });
    }

    void Log::ClearModuleFilter()
    {
        std::lock_guard<std::mutex> lock(m_filterMutex);
        m_moduleFilters.clear();
    }

    void Log::ClearAllFilter()
    {
        ClearAndFilter();
        ClearOrFilter();
        ClearModuleFilter();
    }

    bool Log::IsLogQueEmpty() const
    {
        return m_logQue.empty();
    }

    bool Log::NeedFilter(int module)
    {
        if (m_moduleFilters.empty()) {
            return false;
        }

        std::lock_guard<std::mutex> lock(m_filterMutex);
        auto itr = m_moduleFilters.find(module);
        return m_reverseFilter ? itr == m_moduleFilters.end() : itr != m_moduleFilters.end();
    }

    bool Log::NeedFilterWithAndRule(const std::string& msg)
    {
        if (m_andFilters.empty()) {
            return false;
        }

        bool found = false;
        std::unique_lock<std::mutex> lock(m_filterMutex);
        for (const std::string& filter : m_andFilters) {
            if (msg.find(filter) == std::string::npos) {
                lock.unlock();
                found = true;
                break;
            }
        }

        return m_reverseFilter ? found : !found;
    }

    bool Log::NeedFilterWithOrRule(const std::string& msg)
    {
        if (m_orFilters.empty()) {
            return false;
        }

        bool found = false;
        std::unique_lock<std::mutex> lock(m_filterMutex);
        for (const std::string& filter : m_orFilters) {
            if (msg.find(filter) != std::string::npos) {
                lock.unlock();
                found = true;
                break;
            }
        }

        return m_reverseFilter ? !found : found;
    }

    bool Log::NeedFilter(int module, const std::string& msg)
    {
        return NeedFilter(module) || NeedFilterWithAndRule(msg) || NeedFilterWithOrRule(msg);
    }

    void Log::Write(LogLevel level, int module, const char* fileName, int line, const char* funcName, std::thread::id threadId, const std::string& msg, WriteMode writeMode)
    {
        if (NeedFilter(module, msg)) {
            return;
        }

        std::stringstream ss;
        ss << threadId;
        uint id;
        ss >> id;

        Write(level, writeMode, module, fileName, line, funcName, id, msg);
    }

    void Log::UpdateCurrentDate(const std::string& currentTime)
    {
        std::string date = currentTime.substr(0, 10);
        if (date != m_currentDate) {
            m_currentDate = date;
            m_dateChanged = true;
        }
    }

    std::string Log::FormatLog(LogLevel level, int module, const std::string& fileName, int line, const std::string& funcName, uint threadId, const std::string& info)
    {
        std::string currentTime = GetLocalDateTimeWithMilliSecond();
        UpdateCurrentDate(currentTime);

        if (!m_detailMode) {
            return FORMAT("{} [{}] [{}]: {}", currentTime, LogLevelToStr(level), GetModuleName(module), info);
        }

        return FORMAT("{} [{}] [{}] [{}(line: {}, method: {}, thread: {})]: {}",
            currentTime, LogLevelToStr(level), GetModuleName(module), fileName.substr(fileName.find_last_of(PATH_SEPERATOR) + 1), line, funcName, threadId, info);
    }

    void Log::Write(LogLevel level, WriteMode writeMode, int module, const char* fileName, int line, const char* funcName, uint threadId, const std::string& msg)
    {
        if (m_exit || !IsLogSwitchOn(level)) {
            return;
        }

        std::string str = FormatLog(level, module, fileName, line, funcName, threadId, msg);
        if (writeMode == WriteMode::Newline) {
            str.append("\r\n");
        }

        std::unique_lock<std::mutex> lock(m_queMutex);
        m_logQue.emplace(str);

        return;
    }

    void Log::WritingWorker()
    {
        while (!m_exit) {
            if (m_logQue.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                continue;
            }

            std::string logStr(PopLogStr());

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

        if (!m_colorfulFont) {
            std::unique_lock<std::mutex> lock(m_writeMutex);
            std::cout << msg;
            return;
        }

        std::string coloredMsg = FORMAT("{}{}{}", GetFontColor(msg[25]), msg, FONT_STYLE_CLEAR);
        std::unique_lock<std::mutex> lock(m_writeMutex);
        std::cout << coloredMsg;
    }

    void Log::WriteToLogFile(const std::string& msg)
    {
        if (!IsOutputTypeOn(OutputType::LogFile)) {
            return;
        }

        if (m_dateChanged) {
            m_fileWriter.close();
            m_fileWriter.open(m_logDir + m_currentDate + "_" + m_logFileName, std::ios::out | std::ios::app);
            m_dateChanged = false;
        }

        m_fileWriter << msg;
        m_fileWriter.flush();
    }

    void Log::WriteToUserWriter(const std::string& msg)
    {
        if (!IsOutputTypeOn(OutputType::UserDefined) || m_userWriter == nullptr) {
            return;
        }

        m_userWriter->Write(msg);
    }

    void Log::WriteToRemoteWriter(const std::string& msg)
    {
        if (!IsOutputTypeOn(OutputType::RemoteServer) || m_remoteWriter == nullptr) {
            return;
        }

        m_remoteWriter->Write(msg);
    }

    void Log::SetUserWriter(std::shared_ptr<UserDefinedWriter>& m_fileWriter)
    {
        m_userWriter = m_fileWriter;
    }

    void Log::SetRemoteWriter(std::shared_ptr<UserDefinedWriter>& m_fileWriter)
    {
        m_remoteWriter = m_fileWriter;
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
        std::lock_guard<std::mutex> lock(m_moduleMutex);
        auto itr = m_modulesMap.find(module); 
        return itr != m_modulesMap.end() ? itr->second : "";
    }

    std::string Log::PopLogStr()
    {
        std::lock_guard<std::mutex> lock(m_queMutex);
        std::string logStr = std::move(m_logQue.front());
        m_logQue.pop();
        return logStr;
    }

    void Log::Close()
    {
        if (m_exit) {
            return;
        }

        m_exit = true;

        if (m_writerThread.joinable()) {
            m_writerThread.join();
        }

        m_fileWriter.close();

        if (m_userWriter != nullptr) {
            m_userWriter->Close();
        }

        if (m_remoteWriter != nullptr) {
            m_remoteWriter->Close();
        }
    }

    const char* Log::GetFontColor(char levelFlag)
    {
        switch (levelFlag) {
            case 'D':   // debug level
                return FONT_STYLE_GREEN;
            case 'I':   // info level
                return FONT_STYLE_CYAN;
            case 'W':   // warning level
                return FONT_STYLE_YELLOW;
            case 'E':   // error level
                return FONT_STYLE_RED;
            case 'F':   // fatal level
                return FONT_STYLE_PURPLE;
            default:
                return FONT_STYLE_CLEAR;
        }
    }
}

