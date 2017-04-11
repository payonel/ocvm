#include "log.h"
#include "io/frame.h"

#include <iostream>
#include <functional>
using std::cout;
using std::cerr;
using std::function;

class LogHandler
{
public:
    virtual void write(const string& text)
    {
        Logger::getFrame()->write(1, 1, {text, {}, {}, false, (int)text.size()});
    }
} log_handler;

class LogErrorHandler : public LogHandler
{
public:
    void write(const string& text) override
    {
        _buffer.push_back(text);
        LogHandler::write(text);
    }
    ~LogErrorHandler()
    {
        for (const auto& text : _buffer)
            cerr << text;
    }
private:
    vector<string> _buffer;
} err_handler;

class LogProfHandler : public LogHandler
{
public:
    void write(const string& text) override
    {
        //LogHandler::write(text);
    }
} prof_handler;

Logger::Logger(LogHandler* handler) :
    _handler(handler)
{
}

Logger lout(&log_handler);
Logger lerr(&err_handler);
Logger lprof(&prof_handler);

Frame* Logger::getFrame()
{
    static Frame single_log_frame;
    static bool initialized = false;
    if (!initialized)
    {
        initialized = true;
        single_log_frame.scrolling(true);
    }
    return &single_log_frame;
}

void Logger::handle(const string& text)
{
    _handler->write(text);
}

Logger& operator<< (Logger& logger, std::ostream& (*)(std::ostream&))
{
    logger.handle("\n");
    return logger;
}
