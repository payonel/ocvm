#include "model/log.h"
#include "io/frame.h"
#include "drivers/fs_utils.h"

#include <iostream>
#include <fstream>
#include <functional>
using std::cout;
using std::function;
using std::ofstream;
using std::fstream;

Logger::Logger(const string& logPath) :
    _log_path(logPath)
{
}

void Logger::log_path(const string& path)
{
    _log_path = path;
}

string Logger::log_path() const
{
    return _log_path;
}

Logger& Logger::operator<< (const string& text)
{
    ofstream flog(_log_path, fstream::app);
    flog << text;
    flog.close();
    return *this;
}

Logger& Logger::operator<< (std::ostream& (*)(std::ostream&))
{
    return *this << string("\n");
}

Logger& Logger::operator<< (const char* cstr)
{
    return *this << string(cstr);
}
