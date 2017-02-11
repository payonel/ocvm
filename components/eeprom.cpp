#include "eeprom.h"
#include "log.h"
#include <iostream>
#include <string>
#include <fstream>
#include "utils.h"
#include "host.h"

Eeprom::Eeprom(const Value& config, Host* host) :
    Component(config, host)
{
    add("get", &Eeprom::get);
    add("getData", &Eeprom::getData);
    add("setData", &Eeprom::setData);

    int config_bios_size = config.get(4).toNumber();
    int config_data_size = config.get(5).toNumber();

    _bios_size_limit = config_bios_size == 0 ? _bios_size_limit : config_bios_size;
    _data_size_limit = config_data_size == 0 ? _data_size_limit : config_data_size;

    init(config.get(3).toString());
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

ValuePack Eeprom::setData(const ValuePack& args)
{
    string value = Value::check(args, 0, "string").toString();
    if (value.length() > _data_size_limit)
        return ValuePack({Value::nil, "data size exceeded"});

    return ValuePack({utils::write(value, dataPath())});
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
