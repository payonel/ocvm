#pragma once
#include <string>
#include <sstream>

using std::stringstream;
using std::string;
using std::endl;

class Logger
{
public:
    Logger(string logPath);
    void log_path(const string& path);
    string log_path() const;

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
    string _log_path;
};

extern Logger lout;
