#include "screen.h"

#include <iostream>
#include <vector>

Screen::Screen(const string& type, const Value& init, Host* host) : 
    Component(type, init, host)
{
    add("getKeyboards", &Screen::getKeyboards);
}

ValuePack Screen::getKeyboards(const ValuePack& args)
{
    ValuePack pack({Value::table()});
    Value& list = pack.at(0);
    for (const auto& kb : _keyboards)
    {
        list.insert(kb);
    }
    return pack;
}
