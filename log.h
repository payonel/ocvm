#pragma once
#include <list>
#include <string>
#include <sstream>

using std::stringstream;
using std::string;

class Frame;

class Logger
{
public:
    Logger(int priority);
    static Frame* getFrame();
private:
    Logger();
    int _priority;
};

Logger& operator<< (Logger&, const string& text);
Logger& operator<< (Logger&, std::ostream& (*)(std::ostream&));

template <typename T>
Logger& operator<< (Logger& logger, const T& t)
{
    stringstream ss;
    ss << t;
    logger << ss.str();
    return logger;
}

extern Logger lout;
extern Logger lerr;
