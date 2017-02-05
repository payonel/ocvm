#include "log.h"
using std::string;

#include <iostream>
using std::cout;

Logger lout;
Logger lerr;

Logger::Logger()
{
    scrolling(true);
    setResolution(0, 5);
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

    logger.write(aligned);
    return logger;
}

Logger& operator<< (Logger& logger, std::ostream& (*pf)(std::ostream&))
{
    logger << "\n";
    return logger;
}
