#include "eeprom.h"
#include "log.h"
#include <iostream>
#include <string>
#include <fstream>
#include "utils.h"

Eeprom::Eeprom(const string& type, const Value& args) :
    Component(type, args)
{
    add("get", &Eeprom::get);
    add("getData", &Eeprom::getData);

    init(args.get("env").toString(), args.get(2).toString());
}

void Eeprom::init(const string& dir, const string& originalBiosPath)
{
    if (dir.empty())
    {
        lerr << "bug, eeprom env dir path empty\n";
        return;
    }

    _dir = dir; // now paths work
    if (!utils::read(biosPath()))
    {
        utils::copy(originalBiosPath, biosPath());
    }
}

ValuePack Eeprom::get(const ValuePack& args)
{
    return ValuePack{load(biosPath())};
}

ValuePack Eeprom::getData(const ValuePack& args)
{
    return ValuePack{load(dataPath())};
}

string Eeprom::biosPath() const
{
    return _dir + "/" + "bios.lua";
}

string Eeprom::dataPath() const
{
    return _dir + "/" + "data";
}

string Eeprom::load(const string& path) const
{
    string buffer;
    utils::read(path, &buffer);
    return buffer;
}
