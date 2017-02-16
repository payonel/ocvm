#pragma once

#include "component.h"

class Computer : public Component
{
public:
    Computer();

    int setArchitecture(lua_State* lua);
    int getArchitecture(lua_State* lua);
    int getArchitectures(lua_State* lua);
    int address(lua_State* lua);
    int beep(lua_State* lua);
    int getDeviceInfo(lua_State* lua);
    int getProgramLocations(lua_State* lua);
    int pushSignal(lua_State* lua);
    int removeUser(lua_State* lua);
    int addUser(lua_State* lua);
    int isRobot(lua_State* lua);
    int tmpAddress(lua_State* lua);
    int freeMemory(lua_State* lua);
    int totalMemory(lua_State* lua);
    int energy(lua_State* lua);
    int maxEnergy(lua_State* lua);

    void setTmpAddress(const string& addr);
    void injectCustomLua(lua_State* lua) override;
protected:
    bool onInitialize(Value& config) override;
private:
    static int64_t now();
    int64_t _start_time;
    string _tmp_address;
};
