#pragma once
#include "component.h"
#include "value.h"

class Eeprom : public Component
{
public:
    Eeprom();

    int get(lua_State* lua);
    int getData(lua_State* lua);
    int setData(lua_State* lua);
protected:
    bool onInitialize(Value& config) override;
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
