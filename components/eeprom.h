#pragma once
#include "component.h"
#include "model/value.h"

class Eeprom : public Component
{
public:
    Eeprom();

    enum ConfigIndex
    {
        BiosSize = Component::ConfigIndex::Next,
        DataSize,
        Label
    };

    int get(lua_State* lua);
    int set(lua_State* lua);
    int getData(lua_State* lua);
    int setData(lua_State* lua);
    int getSize(lua_State* lua);
    int getDataSize(lua_State* lua);
    int getLabel(lua_State* lua);
    int setLabel(lua_State* lua);
protected:
    bool onInitialize() override;
    string biosPath() const;
    string dataPath() const;
    vector<char> load(const string& path) const;
    bool postInit() override;
private:
    string _dir; // real on disk storage location

    int _bios_size_limit = 1024 * 4; // also read from configuration
    int _data_size_limit = 256;
};
