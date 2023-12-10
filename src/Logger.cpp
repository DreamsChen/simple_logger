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

#include <fstream>
#include <filesystem>
#include <mutex>
#include <shared_mutex>
#include <queue>
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

    class Log::LogImpl
    {
    public:
        LogImpl(const char* dir, const char* fileName, uint32_t outputFlag, uint32_t logLevelFlag, bool detailMode);
        ~LogImpl();

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

        void SetUserWriter(std::shared_ptr<UserDefinedWriter>& m_userWriter);
        void SetRemoteWriter(std::shared_ptr<UserDefinedWriter>& m_remoteWriter);
        bool IsLogQueEmpty() const;

        void Write(LogLevel level, int module, const char* fileName, int line, const char* funcName, std::thread::id threadId, const std::string& msg, WriteMode writeMode = WriteMode::Newline);

        // Close function should be called munually before the program exit.
        void Close();

    public:
        std::string FormatLog(LogLevel level, int module, std::string_view fileName, int line, std::string_view funcName, uint64_t threadId, const std::string& info);
        void Write(LogLevel level, WriteMode writeMode, int module, const char* fileName, int line, const char* funcName, uint64_t threadId, const std::string& msg);

        bool NeedFilter(int module, const std::string& msg) const;
        bool NeedFilter(int module) const;
        bool NeedFilterWithAndRule(const std::string& msg) const;
        bool NeedFilterWithOrRule(const std::string& msg) const;

        void WriteToConsole(const std::string& msg);
        void WriteToLogFile(const std::string& msg);
        void WriteToUserWriter(const std::string& msg);
        void WriteToRemoteWriter(const std::string& msg);
        const char* LogLevelToStr(LogLevel level);
        void WritingWorker();
        void UpdateCurrentDate(const std::string& currentTime);

        std::string GetModuleName(int module) const;
        const char* GetFontColor(char levelFlag) const;

    private:
        std::string m_logDir;
        std::string m_logFileName;
        std::string m_currentDate;

        uint32_t m_outputFlag;
        uint32_t m_logLevelFlag;

        bool m_detailMode = true;
        bool m_exit = false;
        bool m_dateChanged = false;
        bool m_colorfulFont = true;     // only use in console terminal.
        bool m_reverseFilter = false;   // if m_reverseFilter == true, only the logs that match filters are printed.

        std::queue<std::string> m_logQue;
        std::thread m_writerThread;
        mutable std::mutex m_writeMutex;
        mutable std::mutex m_queMutex;
        mutable std::shared_mutex m_miscMutex;

        std::unordered_map<int, std::string> m_modulesMap;
        std::unordered_set<std::string> m_andFilters;
        std::unordered_set<std::string> m_orFilters;
        std::unordered_set<int> m_moduleFilters;

        std::ofstream m_fileWriter;
        std::shared_ptr<UserDefinedWriter> m_userWriter = nullptr;
        std::shared_ptr<UserDefinedWriter> m_remoteWriter = nullptr;   
    };

    Log::LogImpl::LogImpl(const char* dir, const char* fileName, uint32_t outputFlag, uint32_t logLevelFlag, bool detailMode) :
        m_logDir(dir), m_logFileName(fileName), m_outputFlag(outputFlag), m_logLevelFlag(logLevelFlag), m_detailMode(detailMode)
    {
        m_currentDate = GetLocalDate();
        std::string filePath = m_logDir + "/" + m_currentDate + "_" + fileName;

        if (!std::filesystem::exists(m_logDir)) {
            std::filesystem::create_directory(std::filesystem::path(m_logDir));
        }

        m_fileWriter.open(filePath, std::ios::out | std::ios::app);

        m_writerThread = std::thread(&Log::LogImpl::WritingWorker, this);
    }

    Log::LogImpl::~LogImpl()
    {
        Close();
    }

    uint32_t Log::LogImpl::GetOutputFlag() const
    {
        return m_outputFlag;
    }

    bool Log::LogImpl::IsOutputTypeOn(OutputType type) const
    {
        return (m_outputFlag & ((uint32_t)type)) != 0;
    }

    void Log::LogImpl::SetOutputTypeOn(OutputType outputType)
    {
        m_outputFlag |= (uint32_t)outputType;
    }

    void Log::LogImpl::SetOutputTypeOff(OutputType outputType)
    {
        m_outputFlag &= ~(uint32_t)outputType;
    }

    void Log::LogImpl::DisableLog()
    {
        m_outputFlag = 0;
    }

    bool Log::LogImpl::IsLogSwitchOn(LogLevel level) const
    {
        return (m_logLevelFlag & (uint32_t)level) != 0;
    }

    void Log::LogImpl::SetLogSwitchOn(LogLevel level)
    {
        m_logLevelFlag |= (uint32_t)level;
    }

    void Log::LogImpl::SetLogSwitchOff(LogLevel level)
    {
        m_logLevelFlag &= ~(uint32_t)level;
    }

    void Log::LogImpl::SetDetailMode(bool enable)
    {
        m_detailMode = enable;
    }

    bool Log::LogImpl::IsDetailMode() const
    {
        return m_detailMode;
    }

    void Log::LogImpl::SetColorfulFont(bool enable)
    {
        m_colorfulFont = enable;
    }

    bool Log::LogImpl::IsColorfulFont() const
    {
        return m_colorfulFont;
    }

    void Log::LogImpl::SetReverseFilter(bool enable)
    {
        m_reverseFilter = enable;
    }

    bool Log::LogImpl::IsReverseFilter() const
    {
        return m_reverseFilter;
    }

    void Log::LogImpl::AddModule(int module, const std::string& name)
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        m_modulesMap[module] = name;
    }

    void Log::LogImpl::AddModule(const std::unordered_map<int, std::string>& modules)
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        std::copy(modules.begin(), modules.end(), std::inserter(m_modulesMap, m_modulesMap.end()));
    }

    void Log::LogImpl::RemoveModule(int module)
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        m_modulesMap.erase(module);
    }

    void Log::LogImpl::ClearAllModule()
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        m_modulesMap.clear();
    }

    void Log::LogImpl::AddAndFilter(const std::string& filterString)
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        m_andFilters.insert(filterString);
    }

    void Log::LogImpl::AddAndFilter(const std::unordered_set<std::string>& filterList)
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        std::copy(filterList.begin(), filterList.end(), std::inserter(m_andFilters, m_andFilters.end()));
    }

    void Log::LogImpl::ClearAndFilter(const std::string& filterString)
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        m_andFilters.erase(filterString);
    }

    void Log::LogImpl::ClearAndFilter(const std::unordered_set<std::string>& filterList)
    {
        std::for_each(filterList.begin(), filterList.end(), [this](const std::string& filterString) { ClearAndFilter(filterString); });
    }

    void Log::LogImpl::ClearAndFilter()
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        m_andFilters.clear();
    }

    void Log::LogImpl::AddOrFilter(const std::string& filterString)
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        m_orFilters.insert(filterString);
    }

    void Log::LogImpl::AddOrFilter(const std::unordered_set<std::string>& filterList)
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        std::copy(filterList.begin(), filterList.end(), std::inserter(m_orFilters, m_orFilters.end()));
    }

    void Log::LogImpl::ClearOrFilter(const std::string& filterString)
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        m_orFilters.erase(filterString);
    }

    void Log::LogImpl::ClearOrFilter(const std::unordered_set<std::string>& filterList)
    {
        std::for_each(filterList.begin(), filterList.end(), [this](const std::string& filterString) { ClearOrFilter(filterString); });
    }

    void Log::LogImpl::ClearOrFilter()
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        m_orFilters.clear();
    }

    void Log::LogImpl::AddModuleFilter(int module)
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        m_moduleFilters.insert(module);
    }

    void Log::LogImpl::AddModuleFilter(const std::unordered_set<int>& moduleList)
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        std::copy(moduleList.begin(), moduleList.end(), std::inserter(m_moduleFilters, m_moduleFilters.end()));
    }

    void Log::LogImpl::ClearModuleFilter(int module)
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        m_moduleFilters.erase(module);
    }

    void Log::LogImpl::ClearModuleFilter(const std::unordered_set<int>& moduleList)
    {
        std::for_each(moduleList.begin(), moduleList.end(), [this](int module) { ClearModuleFilter(module); });
    }

    void Log::LogImpl::ClearModuleFilter()
    {
        std::lock_guard<std::shared_mutex> lock(m_miscMutex);
        m_moduleFilters.clear();
    }

    void Log::LogImpl::ClearAllFilter()
    {
        ClearAndFilter();
        ClearOrFilter();
        ClearModuleFilter();
    }

    bool Log::LogImpl::IsLogQueEmpty() const
    {
        return m_logQue.empty();
    }

    bool Log::LogImpl::NeedFilter(int module) const
    {
        if (m_moduleFilters.empty()) {
            return false;
        }

        std::shared_lock<std::shared_mutex> lock(m_miscMutex);
        auto itr = m_moduleFilters.find(module);
        return m_reverseFilter ? itr == m_moduleFilters.end() : itr != m_moduleFilters.end();
    }

    bool Log::LogImpl::NeedFilterWithAndRule(const std::string& msg) const
    {
        if (m_andFilters.empty()) {
            return false;
        }

        bool found = false;
        std::shared_lock<std::shared_mutex> lock(m_miscMutex);
        for (const std::string& filter : m_andFilters) {
            if (msg.find(filter) == std::string::npos) {
                lock.unlock();
                found = true;
                break;
            }
        }

        return m_reverseFilter ? found : !found;
    }

    bool Log::LogImpl::NeedFilterWithOrRule(const std::string& msg) const
    {
        if (m_orFilters.empty()) {
            return false;
        }

        bool found = false;
        std::shared_lock<std::shared_mutex> lock(m_miscMutex);
        for (const std::string& filter : m_orFilters) {
            if (msg.find(filter) != std::string::npos) {
                lock.unlock();
                found = true;
                break;
            }
        }

        return m_reverseFilter ? !found : found;
    }

    bool Log::LogImpl::NeedFilter(int module, const std::string& msg) const
    {
        return NeedFilter(module) || NeedFilterWithAndRule(msg) || NeedFilterWithOrRule(msg);
    }

    void Log::LogImpl::Write(LogLevel level, int module, const char* fileName, int line, const char* funcName, std::thread::id threadId, const std::string& msg, WriteMode writeMode)
    {
        if (NeedFilter(module, msg)) {
            return;
        }

        std::stringstream ss;
        ss << threadId;
        uint64_t id = 0;
        ss >> id;

        Write(level, writeMode, module, fileName, line, funcName, id, msg);
    }

    void Log::LogImpl::UpdateCurrentDate(const std::string& currentTime)
    {
        if (currentTime[9] != m_currentDate[9]) {
            m_currentDate = currentTime.substr(0, 10);
            m_dateChanged = true;
        }
    }

    std::string Log::LogImpl::FormatLog(LogLevel level, int module, const std::string_view fileName, int line, const std::string_view funcName, uint64_t threadId, const std::string& info)
    {
        std::string currentTime = GetLocalDateTimeWithMilliSecond();
        UpdateCurrentDate(currentTime);

        if (!m_detailMode) {
            return FORMAT("{} [{}] [{}]: {}", currentTime, LogLevelToStr(level), GetModuleName(module), info);
        }

        return FORMAT("{} [{}] [{}] [{}(line: {}, method: {}, thread: {})]: {}",
            currentTime, LogLevelToStr(level), GetModuleName(module), fileName.substr(fileName.find_last_of(PATH_SEPERATOR) + 1), line, funcName, threadId, info);
    }

    void Log::LogImpl::Write(LogLevel level, WriteMode writeMode, int module, const char* fileName, int line, const char* funcName, uint64_t threadId, const std::string& msg)
    {
        if (m_exit || !IsLogSwitchOn(level)) {
            return;
        }

        std::string&& str = FormatLog(level, module, fileName, line, funcName, threadId, msg);
        if (writeMode == WriteMode::Newline) {
            str.append("\r\n");
        }

        std::unique_lock<std::mutex> lock(m_queMutex);
        m_logQue.emplace(str);

        return;
    }

    void Log::LogImpl::WritingWorker()
    {
        std::unique_lock<std::mutex> locker(m_queMutex, std::defer_lock);
        while (!m_exit) {
            if (m_logQue.empty()) {
                std::this_thread::sleep_for(std::chrono::milliseconds(300));
                continue;
            }

            locker.lock();
            std::string logStr = std::move(m_logQue.front());
            m_logQue.pop();
            locker.unlock();

            // Only one writing thread, no need to lock for the below action.
            WriteToConsole(logStr);
            WriteToLogFile(logStr);
            WriteToUserWriter(logStr);
            WriteToRemoteWriter(logStr);
        }
    }

    void Log::LogImpl::WriteToConsole(const std::string& msg)
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

    void Log::LogImpl::WriteToLogFile(const std::string& msg)
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
    }

    void Log::LogImpl::WriteToUserWriter(const std::string& msg)
    {
        if (!IsOutputTypeOn(OutputType::UserDefined) || m_userWriter == nullptr) {
            return;
        }

        m_userWriter->Write(msg);
    }

    void Log::LogImpl::WriteToRemoteWriter(const std::string& msg)
    {
        if (!IsOutputTypeOn(OutputType::RemoteServer) || m_remoteWriter == nullptr) {
            return;
        }

        m_remoteWriter->Write(msg);
    }

    void Log::LogImpl::SetUserWriter(std::shared_ptr<UserDefinedWriter>& m_fileWriter)
    {
        m_userWriter = m_fileWriter;
    }

    void Log::LogImpl::SetRemoteWriter(std::shared_ptr<UserDefinedWriter>& m_fileWriter)
    {
        m_remoteWriter = m_fileWriter;
    }

    const char* Log::LogImpl::LogLevelToStr(LogLevel level)
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

    std::string Log::LogImpl::GetModuleName(int module) const
    {
        std::shared_lock<std::shared_mutex> lock(m_miscMutex);
        auto itr = m_modulesMap.find(module); 
        return itr != m_modulesMap.end() ? itr->second : "";
    }

    void Log::LogImpl::Close()
    {
        if (m_exit) {
            return;
        }

        m_exit = true;

        if (m_writerThread.joinable()) {
            m_writerThread.join();
        }

        m_fileWriter.flush();
        m_fileWriter.close();

        if (m_userWriter != nullptr) {
            m_userWriter->Close();
        }

        if (m_remoteWriter != nullptr) {
            m_remoteWriter->Close();
        }
    }

    const char* Log::LogImpl::GetFontColor(char levelFlag) const
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


    // Log public function implementation.
    Log::Log(const char* dir, const char* fileName, uint32_t outputFlag, uint32_t logLevelFlag, bool detailMode) :
        m_impl(std::make_unique<Log::LogImpl>(dir, fileName, outputFlag, logLevelFlag, detailMode))
    {
    }

    Log::Log(std::string_view dir, std::string_view fileName, uint32_t outputFlag, uint32_t logLevelFlag, bool detailMode) :
        m_impl(std::make_unique<Log::LogImpl>(dir.data(), fileName.data(), outputFlag, logLevelFlag, detailMode))
    {
    }

    Log::~Log()
    {
        Close();
    }

    uint32_t Log::Log::GetOutputFlag() const
    {
        return m_impl->GetOutputFlag();
    }

    bool Log::IsOutputTypeOn(OutputType type) const
    {
        return m_impl->IsOutputTypeOn(type);
    }

    void Log::SetOutputTypeOn(OutputType outputType)
    {
        m_impl->SetOutputTypeOn(outputType);
    }

    void Log::SetOutputTypeOff(OutputType outputType)
    {
        m_impl->SetOutputTypeOff(outputType);
    }

    void Log::DisableLog()
    {
        m_impl->DisableLog();
    }

    bool Log::IsLogSwitchOn(LogLevel level) const
    {
        return m_impl->IsLogSwitchOn(level);
    }

    void Log::SetLogSwitchOn(LogLevel level)
    {
        m_impl->SetLogSwitchOn(level);
    }

    void Log::SetLogSwitchOff(LogLevel level)
    {
        m_impl->SetLogSwitchOff(level);
    }

    void Log::SetDetailMode(bool enable)
    {
        m_impl->SetDetailMode(enable);
    }

    bool Log::IsDetailMode() const
    {
        return m_impl->IsDetailMode();
    }

    void Log::SetColorfulFont(bool enable)
    {
        m_impl->SetColorfulFont(enable);
    }

    bool Log::IsColorfulFont() const
    {
        return m_impl->IsColorfulFont();
    }

    void Log::SetReverseFilter(bool enable)
    {
        m_impl->SetReverseFilter(enable);
    }

    bool Log::IsReverseFilter() const
    {
        return m_impl->IsReverseFilter();
    }

    void Log::AddModule(int module, const std::string& name)
    {
        m_impl->AddModule(module, name);
    }

    void Log::AddModule(const std::unordered_map<int, std::string>& modules)
    {
        m_impl->AddModule(modules);
    }

    void Log::RemoveModule(int module)
    {
        m_impl->RemoveModule(module);
    }

    void Log::ClearAllModule()
    {
        m_impl->ClearAllModule();
    }

    void Log::AddAndFilter(const std::string& filterString)
    {
        m_impl->AddAndFilter(filterString);
    }

    void Log::AddAndFilter(const std::unordered_set<std::string>& filterList)
    {
        m_impl->AddAndFilter(filterList);
    }

    void Log::ClearAndFilter(const std::string& filterString)
    {
        m_impl->ClearAndFilter(filterString);
    }

    void Log::ClearAndFilter(const std::unordered_set<std::string>& filterList)
    {
        m_impl->ClearAndFilter(filterList);
    }

    void Log::ClearAndFilter()
    {
        m_impl->ClearAndFilter();
    }

    void Log::AddOrFilter(const std::string& filterString)
    {
        m_impl->AddOrFilter(filterString);
    }

    void Log::AddOrFilter(const std::unordered_set<std::string>& filterList)
    {
        m_impl->AddOrFilter(filterList);
    }

    void Log::ClearOrFilter(const std::string& filterString)
    {
        m_impl->ClearOrFilter(filterString);
    }

    void Log::ClearOrFilter(const std::unordered_set<std::string>& filterList)
    {
        m_impl->ClearOrFilter(filterList);
    }

    void Log::ClearOrFilter()
    {
        m_impl->ClearOrFilter();
    }

    void Log::AddModuleFilter(int module)
    {
        m_impl->AddModuleFilter(module);
    }

    void Log::AddModuleFilter(const std::unordered_set<int>& moduleList)
    {
        m_impl->AddModuleFilter(moduleList);
    }

    void Log::ClearModuleFilter(int module)
    {
        m_impl->ClearModuleFilter(module);
    }

    void Log::ClearModuleFilter(const std::unordered_set<int>& moduleList)
    {
        m_impl->ClearModuleFilter(moduleList);
    }

    void Log::ClearModuleFilter()
    {
        m_impl->ClearModuleFilter();
    }

    void Log::ClearAllFilter()
    {
        m_impl->ClearAllFilter();
    }

    void Log::SetUserWriter(std::shared_ptr<UserDefinedWriter> userWriter)
    {
        m_impl->SetUserWriter(userWriter);
    }

    void Log::SetRemoteWriter(std::shared_ptr<UserDefinedWriter> remoteWriter)
    {
        m_impl->SetRemoteWriter(remoteWriter);
    }

    bool Log::IsLogQueEmpty() const
    {
        return m_impl->IsLogQueEmpty();
    }

    void Log::Write(LogLevel level, int module, const char* fileName, int line, const char* funcName, std::thread::id threadId, const std::string& msg, WriteMode writeMode)
    {
        m_impl->Write(level, module, fileName, line, funcName, threadId, msg, writeMode);
    }

    void Log::Close()
    {
        return m_impl->Close();
    }
}

