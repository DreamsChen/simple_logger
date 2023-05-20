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

#include <iostream>
#include <filesystem>
#include "Logger.h"
#include "DateTime.h"

using namespace std;
using namespace simple_logger;

enum class Module 
{
    Example = 0,
};

class ExampleContext
{
public:
    static ExampleContext& GetInstance()
    {
        static ExampleContext context;
        static auto callOnce = [&]() -> int {
            // once call code
            context.m_log.AddModule(context.GetModuleValue(), context.GetModuleName());
            return 0;
        }();

        return context;
    }

    inline Log& GetLogger()
    {
        return m_log;
    }

    inline Module GetModuleType() const
    {
        return Module::Example;
    }

    inline std::string GetModuleName() const
    {
        return "Module::Example";
    }

    inline int GetModuleValue() const
    {
        return static_cast<int>(Module::Example);
    }

private:
    ExampleContext() : m_log(filesystem::current_path().generic_string(), "test.log") {};
    ExampleContext(const ExampleContext&) = delete;
    ExampleContext(ExampleContext&&) = delete;
    ExampleContext& operator=(const ExampleContext&) = delete;
    ExampleContext& operator=(ExampleContext&&) = delete;
    ~ExampleContext() = default;

    Log m_log;
};

// define print micro of module level to simplify usage
#define EXAMPLE_DEBUG(fmt, ...) DBG_DEBUG(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
#define EXAMPLE_INFO(fmt, ...) DBG_INFO(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
#define EXAMPLE_WARN(fmt, ...) DBG_WARN(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
#define EXAMPLE_ERROR(fmt, ...) DBG_ERROR(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
#define EXAMPLE_FATAL(fmt, ...) DBG_FATAL(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))

int main()
{
    class MyWriter : public UserDefinedWriter
    {
    public:
        virtual ~MyWriter()
        {
            cout << "Destructor MyWriter." << endl;
        }

    public:
        virtual void Write(const char* str) override
        {
            cout << "UserDefinedWriter: " << str;
        }
    };


    Log& log = ExampleContext::GetInstance().GetLogger();
    //log.SetOutputTypeOn(OutputType::Console);
    //log.SetOutputTypeOff(OutputType::Console);
    log.SetLogSwitchOn(LogLevel::Debug);
    log.SetDetailMode(true);
    MyWriter* writer = new MyWriter();
    log.SetUserWriter(writer);

    int loopCount = 100;
    thread t1([&log, loopCount]() {
        for (int i = 0; i < loopCount; ++i) {
            EXAMPLE_DEBUG("@@@@@@T1: hello logger, i={}", i);
            EXAMPLE_INFO("@@@@@@T1: hello logger, i={}", i);
            EXAMPLE_WARN("@@@@@@T1: hello logger, i={}", i);
            EXAMPLE_ERROR("@@@@@@T1: hello logger, i={}", string("sss"));
            EXAMPLE_FATAL("@@@@@@T1: hello logger, i={}", 3.1415);
        }
        });

    thread t2([&log, loopCount]() {
        for (int i = 0; i < loopCount; ++i) {
            EXAMPLE_DEBUG("******T2: hello logger, i={}", i);
            EXAMPLE_INFO("******T2: hello logger, i={}", i);
            EXAMPLE_WARN("******T2: hello logger, i={}", i);
            EXAMPLE_ERROR("******T2: hello logger, i={}", string("sss"));
            EXAMPLE_FATAL("******T2: hello logger, i={}", 3.1415);
        }
        });

    t1.join();
    t2.join();

    while (!log.IsLogQueEmpty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

