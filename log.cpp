#include "log.h"
#include "framing/frame.h"

#include <iostream>
#include <list>
using std::cout;
using std::list;

Logger lout(1);
Logger lerr(0);

static const int LOG_DUMP_SIZE = 10000;

class LogFrame : public Frame
{
public:
    LogFrame()
    {
        scrolling(true);
    }
    ~LogFrame()
    {
        for (auto part : _rolling_buffer)
        {
            cout << part;
        }
        _rolling_buffer.clear();
    }
    void write(const string& text) override
    {
        _rolling_buffer.push_back(text);
        if (_rolling_buffer.size() > LOG_DUMP_SIZE)
        {
            _rolling_buffer.remove(_rolling_buffer.front());
        }
        Frame::write(text);
    }
private:
    list<string> _rolling_buffer;
} single_log_frame;

Frame* Logger::getFrame()
{
    return &single_log_frame;
}

Logger::Logger(int priority) :
    _priority(priority)
{
}

Logger& operator<< (Logger& logger, const string& text)
{
    string aligned = "";

    for (size_t start = 0; start < text.size(); )
    {
        size_t next_newline = text.find("\n", start);
        if (next_newline == string::npos)
        {
            aligned += text.substr(start);
            break;
        }

        aligned += text.substr(start, next_newline - start);
        aligned += "\n\r";
        start = next_newline + 1;
    }

    Logger::getFrame()->write(aligned);
    return logger;
}

Logger& operator<< (Logger& logger, std::ostream& (*pf)(std::ostream&))
{
    logger << "\n";
    return logger;
}
