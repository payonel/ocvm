#include "model/log.h"
#include "io/frame.h"

#include <iostream>
#include <fstream>
#include <functional>
using std::cout;
using std::function;
using std::ofstream;
using std::fstream;

LoggerContext Logger::s_context { "" };
Logger& Logging::lout = Logger::getSingleLogger();

const std::string log_file_name = "log";

void Logger::context(LoggerContext ctx)
{
    s_context = ctx;
}

LoggerContext Logger::context()
{
    return s_context;
}

Logger& Logger::getSingleLogger()
{
    static Logger the_one;
    return the_one;
}

Logger& Logger::operator<< (const string& text)
{
    ofstream flog;
    if (s_context.path.empty())
    {
        std::cerr << "[no log ctx]" << text;
    }
    else
    {
        flog.open(s_context.path + "/" + log_file_name, fstream::app);
        if (flog)
        {
            flog << text;
            flog.close();
        }
        else
        {
            std::cerr << "[log write failure]" << text;
        }
    }
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
