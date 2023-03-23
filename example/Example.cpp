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

using namespace std;
using namespace simple_logger;

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

    string dir{ filesystem::current_path().generic_string()};
    string fileName{ "test.log" };
    Log log(dir, fileName, static_cast<uint>(OutputType::UserDefined) | static_cast<uint>(OutputType::LogFile), true);
    MyWriter* writer = new MyWriter();
    log.SetUserWriter(writer);

    int loopCount = 100;
    thread t1([&log, loopCount]() {
        for (int i = 0; i < loopCount; ++i) {
            DBG_TRACE(log, "@@@@@@T1: hello logger, i={}", i);
            DBG_INFO(log, "@@@@@@T1: hello logger, i={}", i);
            DBG_WARN(log, "@@@@@@T1: hello logger, i={}", i);
            DBG_ERROR(log, "@@@@@@T1: hello logger, i={}", string("sss"));
            DBG_FATAL(log, "@@@@@@T1: hello logger, i={}", 3.1415);
        }
        });

    thread t2([&log, loopCount]() {
        for (int i = 0; i < loopCount; ++i) {
            DBG_TRACE(log, "******T2: hello logger, i={}", i);
            DBG_INFO(log, "******T2: hello logger, i={}", i);
            DBG_WARN(log, "******T2: hello logger, i={}", i);
            DBG_ERROR(log, "******T2: hello logger, i={}", string("sss"));
            DBG_FATAL(log, "******T2: hello logger, i={}", 3.1415);
        }
        });

    t1.join();
    t2.join();

    while (!log.IsLogQueEmpty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

