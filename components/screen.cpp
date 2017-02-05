#include "screen.h"

#include <iostream>
using std::cout;
using std::endl;
using std::flush;

Screen::Screen(const Value& value)
{
}

void Screen::test()
{
}

void Screen::invoke(const std::string& methodName)
{
    if (methodName == "test")
        this->test();
}
