#include "log.h"
#include "framing/frame.h"

#include <iostream>
using std::cout;

Logger lout(1);
Logger lerr(0);

class LogFrame : public Frame
{
public:
    LogFrame()
    {
        scrolling(true);
    }
    ~LogFrame()
    {
    }
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
