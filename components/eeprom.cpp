#include "eeprom.h"
#include "log.h"
#include <iostream>
#include <string>
#include <fstream>
#include "utils.h"
#include "host.h"

Eeprom::Eeprom(const string& type, const Value& args, Host* host) :
    Component(type, args, host)
{
    add("get", &Eeprom::get);
    add("getData", &Eeprom::getData);

    init(args.get(2).toString());
}

void Eeprom::init(const string& originalBiosPath)
{
    if (host()->envPath().empty())
    {
        lerr << "bug, eeprom env dir path empty\n";
        return;
    }

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
    return host()->envPath() + "/bios.lua";
}

string Eeprom::dataPath() const
{
    return host()->envPath() + "/data";
}

string Eeprom::load(const string& path) const
{
    string buffer;
    utils::read(path, &buffer);
    return buffer;
}
