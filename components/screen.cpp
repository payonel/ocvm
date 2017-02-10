#include "screen.h"

#include <iostream>
#include <vector>

Screen::Screen(const string& type, const Value& init, Host* host) : 
    Component(type, init, host)
{
}
