#include "eeprom.h"
#include "model/log.h"
#include <iostream>
#include <string>
#include <fstream>
#include "drivers/fs_utils.h"
#include "model/client.h"

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

    if (!fs_utils::read(biosPath()))
    {
        lout << "no computer eeprom found, copying from system\n";
        if (!fs_utils::copy(originalBiosPath, biosPath()))
        {
            lerr << "Could not create an initial bios from: " << originalBiosPath << endl;
            return false;
        }
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
    static const vector<char> default_value {};
    vector<char> value = Value::checkArg<vector<char>>(lua, 1, &default_value);
    size_t len = value.size();
    if (_data_size_limit < 0 || len > static_cast<size_t>(_data_size_limit))
        return ValuePack::ret(lua, Value::nil, "data size exceeded");

    return ValuePack::ret(lua, fs_utils::write(value, dataPath()));
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
    fs_utils::read(path, &buffer);
    return buffer;
}

int Eeprom::getLabel(lua_State* lua)
{
    return ValuePack::ret(lua, config().get(ConfigIndex::Label));
}

int Eeprom::setLabel(lua_State* lua)
{
    update(ConfigIndex::Label, Value::checkArg<string>(lua, 1));
    return 0;
}
