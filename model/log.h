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

    Logger& operator<< (const string& text);
    Logger& operator<< (std::ostream& (*)(std::ostream&));
    Logger& operator<< (const char* cstr);
    template <typename T>
    Logger& operator<< (const T& t)
    {
        stringstream ss;
        ss << t;
        return *this << ss.str();
    }
private:
    LogHandler* _handler;
};

extern Logger lout;
extern Logger lerr;
