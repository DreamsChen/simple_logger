Simple logger lib writen with C++
=========================
- Author: Dreams chen
- First release: 2023-03-23

## Sumary
#

This is a simple logger lib writen with C++, it is small, very easy to used, and fast logger, it can also be used in multiple threads environment. 

## Features
#
- Five log levels: debug, info, warn, error, fatal. Each level can be turned on or off by api. The debug level do nothing by default, other levels will output log content to the log terminal. 
- Four output types: console, file, user defined writer and remote writer(can be used in network environment). Each output type can be turned on or off by api.
- In detail mode, the module name, file name, line number, function name, and thread id can be show in log content, it is easy to identify where the log happen.
- Support to use in multiple threads environment.
- Support C++20's formatted function, it is modern formatter, convenient to print multiple parameters. If not support C++20's formatted function in user's compliler environment, the alternative formatted function is available, can be used in similary usage(but fewer functions, only the simplest function is support).
- Support multiple log filters, include module filters, AND filters, OR filters.
- Support colorful font when logs are printted in console.
- Writen with standard C++, supply multiple os platforms, include linux, windows, etc.

## Examples
#
- Use simple logger 
```
int main()
{
    Log log(filesystem::current_path().generic_string(), "test.log");
    log.SetOutputTypeOn(OutputType::Console);
    log.SetLogSwitchOn(LogLevel::Debug);

    LOG_TRACE(log, 0, "hello logger, intVal={}", 1);
    LOG_INFO(log, 0, "hello logger, doubleVal={}", 3.14);
    LOG_WARN(log, 0, "hello logger, stringVal={}", "simple logger");
    LOG_ERROR(log, 0, "hello logger, stringVal={}", string("std::string"));
    LOG_FATAL(log, 0, "hello logger");

    while (!log.IsLogQueEmpty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
```
In console terminal, it looks like the below:
![](https://github.com/DreamsChen/simple_logger/blob/main/res/print2console.png?raw=true)


- Use simple logger in multiple threads environment
```

int main()
{
    Log log(filesystem::current_path().generic_string(), "test.log");

    int loopCount = 100;
    thread t1([&log, loopCount]() {
        for (int i = 0; i < loopCount; ++i) {
            LOG_TRACE(log, 0, "@@@@@@T1: hello logger, i={}", i);
            LOG_INFO(log, 0, "@@@@@@T1: hello logger, i={}", i);
            LOG_WARN(log, 0, "@@@@@@T1: hello logger, i={}", i);
            LOG_ERROR(log, 0, "@@@@@@T1: hello logger, i={}", i);
            LOG_FATAL(log, 0, "@@@@@@T1: hello logger, i={}", i);
        }
        });

    thread t2([&log, loopCount]() {
        for (int i = 0; i < loopCount; ++i) {
            LOG_TRACE(log, 0, "******T2: hello logger, i={}", i);
            LOG_INFO(log, 0, "******T2: hello logger, i={}", i);
            LOG_WARN(log, 0, "******T2: hello logger, i={}", i);
            LOG_ERROR(log, 0, "******T2: hello logger, i={}", i);
            LOG_FATAL(log, 0, "******T2: hello logger, i={}", i);
        }
        });
        
    t1.join();
    t2.join();
    
    while (!log.IsLogQueEmpty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }    
}
```

- Output to multiple log terminals
```
int main()
{
    Log log(filesystem::current_path().generic_string(), "test.log");
    log.SetOutputTypeOn(OutputType::Console);
    log.SetOutputTypeOn(OutputType::LogFile);
    log.SetLogSwitchOn(LogLevel::Debug);

    LOG_TRACE(log, 0, "hello logger, intVal={}", 1);
    LOG_INFO(log, 0, "hello logger, doubleVal={}", 3.14);
    LOG_WARN(log, 0, "hello logger, stringVal={}", "simple logger");
    LOG_ERROR(log, 0, "hello logger, stringVal={}", string("std::string"));
    LOG_FATAL(log, 0, "hello logger");

    while (!log.IsLogQueEmpty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}
```

- Use simple logger with user defined writer in QT GUI application
```
enum class Module 
{
    App = 0,
};

class AppContext
{
public:
    static AppContext& GetInstance()
    {
        static AppContext context;
        static auto callOnce = [&]() -> int {
            // once call code here
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
        return Module::App;
    }

    inline std::string GetModuleName() const
    {
        return "Module::App";
    }

    inline int GetModuleValue() const
    {
        return static_cast<int>(Module::App);
    }

private:
    AppContext() : m_log(filesystem::current_path().generic_string(), "app.log") {};
    AppContext(const AppContext&) = delete;
    AppContext(AppContext&&) = delete;
    AppContext& operator=(const AppContext&) = delete;
    AppContext& operator=(AppContext&&) = delete;
    ~AppContext() { m_log.Close(); }

private:
    simple_logger Log m_log;
};

class TextBoxWriter : public QObject, public UserDefinedWriter
{
    Q_OBJECT;

public:
    TextBoxWriter(MainWindow* window) : m_window(window) {}
    virtual ~TextBoxWriter() { m_window = nullptr; }

public:
    virtual void Write(const string& str) override
    {
        if (m_window != nullptr) {
            emit ReceiveNewLog(QString(str.c_str()));
        }
    }

    virtual void Close() override
    {
        m_window = nullptr;
    }

signals:
    void ReceiveNewLog(const QString& logStr);

private:
    MainWindow* m_window;
};

// define print micro of module level to simplify usage
#define APP_DEBUG(fmt, ...) DBG_DEBUG(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
#define APP_INFO(fmt, ...) DBG_INFO(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
#define APP_WARN(fmt, ...) DBG_WARN(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
#define APP_ERROR(fmt, ...) DBG_ERROR(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
#define APP_FATAL(fmt, ...) DBG_FATAL(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))

// use textbox writer in MainWindow class
class TextBoxWriter;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget* parent = nullptr);
    ~MainWindow();

private slots:
    void OnReceiveNewLog(const QString str);

private:
    Ui::MainWindow* ui;
    std::shared_ptr<TextBoxWriter> m_textboxWriter;
    simple_logger::Log& m_log;
};

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
, ui(new Ui::MainWindow)
, m_textboxWriter(std::make_shared<TextBoxWriter>(this))
, m_log(AppContext::GetInstance().GetLogger())
{
    
    connect(m_textboxWriter.get(), &TextBoxWriter::ReceiveNewLog, this, &MainWindow::OnReceiveNewLog, Qt::ConnectionType::QueuedConnection);
    ui->setupUi(this);
    ui->setupUi(this);

    // connect signal slot
    connect(m_textboxWriter.get(), &TextBoxWriter::ReceiveNewLog, this, &MainWindow::OnReceiveNewLog, Qt::ConnectionType::QueuedConnection);
    m_log.SetUserWriter(m_textboxWriter);

    APP_TRACE("hello logger, intVal={}", 1);
    APP_INFO("hello logger, doubleVal={}", 3.14);
    APP_WARN("hello logger, stringVal={}", "simple logger");
    APP_ERROR("hello logger, stringVal={}", string("std::string"));
    APP_FATAL("hello logger");
}
 
void MainWindow::OnReceiveNewLog(const QString str)
{
    // outputWindow is a QTextEdit component
    ui->outputWindow->append(str);
}

```
