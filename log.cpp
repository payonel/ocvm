#include "log.h"
using std::string;

#include <iostream>
using std::cout;

Logger& operator<< (Logger& lout, const string& text)
{
    cout << text;
    return lout;
}
