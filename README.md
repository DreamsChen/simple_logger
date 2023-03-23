Simple logger lib writen by C++
=========================
- author：Dreams chen
- first release: 2023年3月

## Sumary
#

This is a simple logger lib writen by C++, it is small, very easy to used, and fast logger, and can be used in multiple threads environment. 

## Features
#
- Five log levels: debug, info, warn, error, fatal. Each level can be turned on or off by api. The debug level do nothing by default, other levels will output log content to the log terminal. 
- Three output types: console, file, user defined writer. Each output type can be turned on or off by api.
- In detail mode, the file name, line number, and function name can be show in log content, it is easy to identify where the log happen.
- Support multiple thread.
- Support C++20's formatted function, it is modern formatter, convenient to print multiple parameters. If not support C++20's formatted function in user complile environment, the alternative formatted function is available, can be used in similary usage(but fewer functions, only the simplest function is support).
- Writen with standard C++, supply multiple os platforms, include linux, windows, etc.

## Examples
#
- Use simple logger 
```
int main()
{
    string dir{ filesystem::current_path().generic_string() };
    string fileName{ "test.log" };
    Log log(dir, fileName, static_cast<uint>(OutputType::Console), true);
    log.SetLogSwitchOn(LogLevel::Debug);

    DBG_TRACE(log, "hello logger, intVal={}", 1);
    DBG_INFO(log, "hello logger, doubleVal={}", 3.14);
    DBG_WARN(log, "hello logger, stringVal={}", "simple logger");
    DBG_ERROR(log, "hello logger, stringVal={}", string("std::string"));
    DBG_FATAL(log, "hello logger");

    while (!log.IsLogQueEmpty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
```

- Use simple logger with user defined writer
```
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

int main()
{
    string dir{ filesystem::current_path().generic_string()};
    string fileName{ "test.log" };
    Log log(dir, fileName, static_cast<uint>(OutputType::UserDefined), true);
    MyWriter* writer = new MyWriter();
    log.SetUserWriter(writer);

    DBG_TRACE(log, "hello logger, intVal={}", 1);
    DBG_INFO(log, "hello logger, doubleVal={}", 3.14);
    DBG_WARN(log, "hello logger, stringVal={}", "simple logger");
    DBG_ERROR(log, "hello logger, stringVal={}", string("std::string"));
    DBG_FATAL(log, "hello logger");

    while (!log.IsLogQueEmpty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
```

- Output to multiple log terminals
```
int main()
{
    string dir{ filesystem::current_path().generic_string() };
    string fileName{ "test.log" };
    Log log(dir, fileName, static_cast<uint>(OutputType::Console) | static_cast<uint>(OutputType::LogFile), true);
    log.SetLogSwitchOn(LogLevel::Debug);

    DBG_TRACE(log, "hello logger, intVal={}", 1);
    DBG_INFO(log, "hello logger, doubleVal={}", 3.14);
    DBG_WARN(log, "hello logger, stringVal={}", "simple logger");
    DBG_ERROR(log, "hello logger, stringVal={}", string("std::string"));
    DBG_FATAL(log, "hello logger");

    while (!log.IsLogQueEmpty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
```

- Use simple logger in multiple threads environment
```

int main()
{
    string dir{ filesystem::current_path().generic_string()};
    string fileName{ "test.log" };
    Log log(dir, fileName, static_cast<uint>(OutputType::LogFile), true);

    int loopCount = 100;
    thread t1([&log, loopCount]() {
        for (int i = 0; i < loopCount; ++i) {
            DBG_TRACE(log, "@@@@@@T1: hello logger, i={}", i);
            DBG_INFO(log, "@@@@@@T1: hello logger, i={}", i);
            DBG_WARN(log, "@@@@@@T1: hello logger, i={}", i);
            DBG_ERROR(log, "@@@@@@T1: hello logger, i={}", i);
            DBG_FATAL(log, "@@@@@@T1: hello logger, i={}", i);
        }
        });

    thread t2([&log, loopCount]() {
        for (int i = 0; i < loopCount; ++i) {
            DBG_TRACE(log, "******T2: hello logger, i={}", i);
            DBG_INFO(log, "******T2: hello logger, i={}", i);
            DBG_WARN(log, "******T2: hello logger, i={}", i);
            DBG_ERROR(log, "******T2: hello logger, i={}", i);
            DBG_FATAL(log, "******T2: hello logger, i={}", i);
        }
        });
        
    t1.join();
    t2.join();
    
    while (!log.IsLogQueEmpty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }    
}
```