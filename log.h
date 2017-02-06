#pragma once
#include <list>
#include <string>
#include <sstream>

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

Logger& operator<< (Logger&, const std::string& text);
Logger& operator<< (Logger&, std::ostream& (*)(std::ostream&));

template <typename T>
Logger& operator<< (Logger& logger, const T& t)
{
    std::stringstream ss;
    ss << t;
    logger << ss.str();
    return logger;
}

extern Logger lout;
extern Logger lerr;
