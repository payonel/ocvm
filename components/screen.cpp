#include "screen.h"

#include <iostream>
#include <vector>
using std::cout;
using std::endl;
using std::flush;
using std::vector;

Screen::Screen(const std::string& type, const Value& init) : 
    Component(type, init)
{
}

