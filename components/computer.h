#pragma once

#include "component.h"

class Computer : public Component
{
public:
    Computer();

    ValuePack setArchitecture(lua_State* lua);
    ValuePack getArchitecture(lua_State* lua);
    ValuePack getArchitectures(lua_State* lua);
    ValuePack address(lua_State* lua);
    ValuePack beep(lua_State* lua);
    ValuePack getDeviceInfo(lua_State* lua);
    ValuePack getProgramLocations(lua_State* lua);
    ValuePack uptime(lua_State* lua);
    ValuePack pushSignal(lua_State* lua);
    ValuePack removeUser(lua_State* lua);
    ValuePack addUser(lua_State* lua);
    ValuePack isRobot(lua_State* lua);
    ValuePack tmpAddress(lua_State* lua);
    ValuePack freeMemory(lua_State* lua);
    ValuePack totalMemory(lua_State* lua);
    ValuePack energy(lua_State* lua);
    ValuePack maxEnergy(lua_State* lua);

    void setTmpAddress(const string& addr);
    void injectCustomLua(lua_State* lua) override;
protected:
    bool onInitialize(Value& config) override;
private:
    static int64_t now();
    int64_t _start_time;
    string _tmp_address;
};
