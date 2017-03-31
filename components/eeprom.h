#pragma once
#include "component.h"
#include "value.h"

class Eeprom : public Component
{
public:
    Eeprom();

    enum ConfigIndex
    {
        BiosPath = Component::ConfigIndex::Next,
        BiosSize,
        DataSize
    };

    int get(lua_State* lua);
    int getData(lua_State* lua);
    int setData(lua_State* lua);
    int getSize(lua_State* lua);
    int getDataSize(lua_State* lua);
    int getLabel(lua_State* lua);
protected:
    bool onInitialize() override;
    string biosPath() const;
    string dataPath() const;
    string load(const string& path) const;
private:
    string _bios;
    string _data;

    string _dir; // real on disk storage location

    int _bios_size_limit = 1024 * 4; // also read from configuration
    int _data_size_limit = 256;
};
