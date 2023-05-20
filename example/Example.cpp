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
            // once call code here
            context.m_log.AddModule(context.GetModuleValue(), context.GetModuleName());
            return 0;
        }();

        return context;
    }

    inline simple_logger::Log& GetLogger()
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
    ExampleContext() : m_log(std::filesystem::current_path().generic_string(), "test.log") {};
    ExampleContext(const ExampleContext&) = delete;
    ExampleContext(ExampleContext&&) = delete;
    ExampleContext& operator=(const ExampleContext&) = delete;
    ExampleContext& operator=(ExampleContext&&) = delete;
    ~ExampleContext() 
    {
        m_log.Close();
    }

private:
    simple_logger::Log m_log;
};

// define print micro of module level to simplify usage
#define EXAMPLE_DEBUG(fmt, ...) DBG_DEBUG(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
#define EXAMPLE_INFO(fmt, ...) DBG_INFO(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
#define EXAMPLE_WARN(fmt, ...) DBG_WARN(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
#define EXAMPLE_ERROR(fmt, ...) DBG_ERROR(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))
#define EXAMPLE_FATAL(fmt, ...) DBG_FATAL(ExampleContext::GetInstance().GetLogger(), ExampleContext::GetInstance().GetModuleValue(), FORMAT(fmt, ##__VA_ARGS__))


// user defined writer example:
// define a textbox writer in QT GUI application.
// 
//class TextBoxWriter : public QObject, public simple_logger::UserDefinedWriter
//{
//    Q_OBJECT;
//
//public:
//    TextBoxWriter(MainWindow* window) : m_window(window)
//    {
//
//    }
//    virtual ~TextBoxWriter() { m_window = nullptr; }
//
//public:
//    virtual void Write(const std::string& str) override
//    {
//        if (m_window != nullptr) {
//            emit ReceiveNewLog(QString(str.c_str()));
//        }
//    }
//
//    virtual void Close() override
//    {
//        m_window = nullptr;
//    }
//
//signals:
//    void ReceiveNewLog(const QString& logStr);
//
//private:
//    MainWindow* m_window;
//};
//
// use textbox writer on mainwindow class
//
//class TextBoxWriter;
//class MainWindow : public QMainWindow
//{
//    Q_OBJECT
//
//public:
//    MainWindow(QWidget* parent = nullptr);
//    ~MainWindow();
//
//private slots:
//    void OnReceiveNewLog(const QString str);
//
//private:
//    Ui::MainWindow* ui;
//    std::shared_ptr<TextBoxWriter> m_textboxWriter;
//    simple_logger::Log m_log;
//};
//
//MainWindow::MainWindow(QWidget *parent)
//: QMainWindow(parent)
//, ui(new Ui::MainWindow)
//, m_textboxWriter(std::make_shared<TextBoxWriter>(this))
//, m_log(std::filesystem::current_path().string(), "app.log")
//{
//    ui->setupUi(this);
//    connect(m_textboxWriter.get(), &TextBoxWriter::ReceiveNewLog, this, &MainWindow::OnReceiveNewLog, Qt::ConnectionType::QueuedConnection);
//    m_log.SetUserWriter(m_textboxWriter)
//}
// 
//void MainWindow::OnReceiveNewLog(const QString str)
//{
//    ui->outputWindow->append(str);
//}



int main()
{
    class MyWriter : public simple_logger::UserDefinedWriter
    {
    public:
        virtual ~MyWriter()
        {
            std::cout << "Destructor MyWriter." << std::endl;
        }

    public:
        virtual void Write(const std::string& str) override
        {
            std::cout << "UserDefinedWriter: " << str;
        }
    };


    simple_logger::Log& log = ExampleContext::GetInstance().GetLogger();
    log.SetOutputTypeOn(simple_logger::OutputType::Console);
    //log.SetOutputTypeOff(simple_logger::OutputType::Console);
    log.SetLogSwitchOn(simple_logger::LogLevel::Debug);
    log.SetDetailMode(true);

    //std::shared_ptr<simple_logger::UserDefinedWriter> writer = std::make_shared<MyWriter>();
    //log.SetUserWriter(writer);

    int loopCount = 10;
    std::thread t1([&log, loopCount]() {
        for (int i = 0; i < loopCount; ++i) {
            EXAMPLE_DEBUG("######T1: hello logger, i={}", i);
            EXAMPLE_INFO("######T1: hello logger, i={}", i);
            EXAMPLE_WARN("######T1: hello logger, i={}", i);
            EXAMPLE_ERROR("######T1: hello logger, i={}", std::string("sss"));
            EXAMPLE_FATAL("######T1: hello logger, i={}", 3.1415);
        }
        });

    std::thread t2([&log, loopCount]() {
        for (int i = 0; i < loopCount; ++i) {
            EXAMPLE_DEBUG("******T2: hello logger, i={}", i);
            EXAMPLE_INFO("******T2: hello logger, i={}", i);
            EXAMPLE_WARN("******T2: hello logger, i={}", i);
            EXAMPLE_ERROR("******T2: hello logger, i={}", std::string("sss"));
            EXAMPLE_FATAL("******T2: hello logger, i={}", 3.1415);
        }
        });

    t1.join();
    t2.join();

    while (!log.IsLogQueEmpty()) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

