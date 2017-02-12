#include "eeprom.h"
#include "log.h"
#include <iostream>
#include <string>
#include <fstream>
#include "utils.h"
#include "client.h"

Eeprom::Eeprom()
{
    add("get", &Eeprom::get);
    add("getData", &Eeprom::getData);
    add("setData", &Eeprom::setData);
}

bool Eeprom::onInitialize(Value& config)
{
    string originalBiosPath = config.get(3).toString();

    int config_bios_size = config.get(4).toNumber();
    int config_data_size = config.get(5).toNumber();

    _bios_size_limit = config_bios_size == 0 ? _bios_size_limit : config_bios_size;
    _data_size_limit = config_data_size == 0 ? _data_size_limit : config_data_size;

    if (client()->envPath().empty())
    {
        lerr << "bug, eeprom env dir path empty\n";
        return false;
    }

    if (!utils::read(biosPath()))
    {
        return utils::copy(originalBiosPath, biosPath());
    }

    return true;
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
    size_t len = value.length();
    if (_data_size_limit < 0 || len > static_cast<size_t>(_data_size_limit))
        return ValuePack({Value::nil, "data size exceeded"});

    return ValuePack({utils::write(value, dataPath())});
}

string Eeprom::biosPath() const
{
    return client()->envPath() + "/bios.lua";
}

string Eeprom::dataPath() const
{
    return client()->envPath() + "/data";
}

string Eeprom::load(const string& path) const
{
    string buffer;
    utils::read(path, &buffer);
    return buffer;
}
