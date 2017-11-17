#pragma once
#include <string>
#include <sstream>

using std::stringstream;
using std::string;
using std::endl;

class Logger
{
public:
    Logger(const string& logPath = "");
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

    string serialize(stringstream& ss)
    {
        return ss.str() + "\n";
    }

    template <typename T, typename ...Ts>
    string serialize(stringstream& ss, const T& arg, const Ts&... args)
    {
        ss << arg;
        return serialize(ss, args...);
    }

    template <typename ...Ts>
    Logger& write(const Ts&... args)
    {
        stringstream ss;
        return *this << serialize(ss, args...);
    }
private:
    string _log_path;
};
