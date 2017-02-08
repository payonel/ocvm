#include "screen.h"

#include <iostream>
#include <vector>
using std::cout;
using std::endl;
using std::flush;

Screen::Screen(const string& type, const Value& init) : 
    Component(type, init)
{
}
