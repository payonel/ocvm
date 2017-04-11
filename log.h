#pragma once
#include <list>
#include <string>
#include <sstream>

using std::stringstream;
using std::string;
using std::endl;

class Frame;

class LogHandler;
class Logger
{
public:
    Logger(LogHandler* handler);
    
    static Frame* getFrame();
    void handle(const string& text);
private:
    LogHandler* _handler;
};

Logger& operator<< (Logger& logger, std::ostream& (*)(std::ostream&));

template <typename T>
Logger& operator<< (Logger& logger, const T& t)
{
    stringstream ss;
    ss << t;
    logger.handle(ss.str());
    return logger;
}

extern Logger lout;
extern Logger lerr;
extern Logger lprof;
