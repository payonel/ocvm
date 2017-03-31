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
    add("getSize", &Eeprom::getSize);
    add("getDataSize", &Eeprom::getDataSize);
    add("getLabel", &Eeprom::getLabel);
    add("setLabel", &Eeprom::setLabel);
}

bool Eeprom::onInitialize()
{
    string originalBiosPath = config().get(ConfigIndex::BiosPath).toString();

    int config_bios_size = config().get(ConfigIndex::BiosSize).toNumber();
    int config_data_size = config().get(ConfigIndex::DataSize).toNumber();

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

int Eeprom::get(lua_State* lua)
{
    return ValuePack::ret(lua, this->load(biosPath()));
}

int Eeprom::getData(lua_State* lua)
{
    return ValuePack::ret(lua, this->load(dataPath()));
}

int Eeprom::getSize(lua_State* lua)
{
    return ValuePack::ret(lua, _bios_size_limit);
}

int Eeprom::getDataSize(lua_State* lua)
{
    return ValuePack::ret(lua, _data_size_limit);
}

int Eeprom::setData(lua_State* lua)
{
    string value = Value::check(lua, 1, "string", "nil").Or("").toString();
    size_t len = value.length();
    if (_data_size_limit < 0 || len > static_cast<size_t>(_data_size_limit))
        return ValuePack::ret(lua, Value::nil, "data size exceeded");

    return ValuePack::ret(lua, utils::write(value, dataPath()));
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

int Eeprom::getLabel(lua_State* lua)
{
    return ValuePack::ret(lua, config().get(ConfigIndex::Label));
}

int Eeprom::setLabel(lua_State* lua)
{
    update(ConfigIndex::Label, Value::check(lua, 1, "string"));
    return 0;
}
