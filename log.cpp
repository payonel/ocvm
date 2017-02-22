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
        setResolution(1, 1);
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
    Frame* pf = Logger::getFrame();
    pf->set(1, 1, {text, pf->foreground(), pf->background()});
    return logger;
}

Logger& operator<< (Logger& logger, std::ostream& (*pf)(std::ostream&))
{
    logger << "\n";
    return logger;
}
