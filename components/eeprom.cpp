#include "eeprom.h"
#include "log.h"
#include <iostream>
#include <string>
#include <fstream>
#include "utils.h"

Eeprom::Eeprom(const string& type, const Value& init) :
    Component(type, init)
{
    add("get", &Eeprom::get);

    load(init.get("env").toString(), init.get(2).toString());
}

void Eeprom::load(const string& dir, const string& file)
{
    if (dir.empty())
    {
        lerr << "bug, eeprom env dir path empty\n";
        return;
    }

    _path = dir + "/" + file;

    if (!utils::read(_path))
    {
        utils::copy("system/" + file, _path);
    }
}

ValuePack Eeprom::get(const ValuePack& args)
{
    return ValuePack();
}
