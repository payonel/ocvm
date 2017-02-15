#include "computer.h"
#include "log.h"
#include <lua.hpp>

#include <iostream>
#include <chrono>
using namespace std::chrono;

Computer::Computer()
{
    _start_time = now();

    add("setArchitecture", &Computer::setArchitecture);
    add("getArchitecture", &Computer::getArchitecture);
    add("getArchitectures", &Computer::getArchitectures);
    add("beep", &Computer::beep);
    add("getDeviceInfo", &Computer::getDeviceInfo);
    add("getProgramLocations", &Computer::getProgramLocations);
    add("uptime", &Computer::uptime);
    add("pushSignal", &Computer::pushSignal);
    add("removeUser", &Computer::removeUser);
    add("addUser", &Computer::addUser);
    add("isRobot", &Computer::isRobot);
    add("tmpAddress", &Computer::tmpAddress);
    add("freeMemory", &Computer::freeMemory);
    add("totalMemory", &Computer::totalMemory);
    add("energy", &Computer::energy);
    add("maxEnergy", &Computer::maxEnergy);
}

bool Computer::onInitialize(Value& config)
{
    return true;
}

int64_t Computer::now()
{
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

void Computer::setTmpAddress(const string& addr)
{
    _tmp_address = addr;
}

void Computer::injectCustomLua(lua_State* lua)
{
    lua_getglobal(lua, "os"); // +1
    lua_pushstring(lua, "time"); // push key name, +1
    lua_gettable(lua, -2); // push time on stack, pop key name, +1-1
    lua_remove(lua, -2); // pop os, -1
    lua_setfield(lua, -2, "realTime"); // computer.realTime = time, pops time, -1
}

ValuePack Computer::setArchitecture(lua_State* lua)
{
    luaL_error(lua, "setArchitecture not implemented");
    return { };
}

ValuePack Computer::getArchitecture(lua_State* lua)
{
    luaL_error(lua, "getArchitecture not implemented");
    return { };
}

ValuePack Computer::getArchitectures(lua_State* lua)
{
    luaL_error(lua, "getArchitectures not implemented");
    return { };
}

ValuePack Computer::beep(lua_State* lua)
{
    lout << "\a" << "BEEP\n";
    return { };
}

ValuePack Computer::getDeviceInfo(lua_State* lua)
{
    luaL_error(lua, "getDeviceInfo not implemented");
    return { };
}

ValuePack Computer::getProgramLocations(lua_State* lua)
{
    luaL_error(lua, "getProgramLocations not implemented");
    return { };
}

// in seconds
ValuePack Computer::uptime(lua_State* lua)
{
    return ValuePack({now() - _start_time});
}

ValuePack Computer::pushSignal(lua_State* lua)
{
    return { };
}

ValuePack Computer::removeUser(lua_State* lua)
{
    luaL_error(lua, "removeUser not implemented");
    return { };
}

ValuePack Computer::addUser(lua_State* lua)
{
    luaL_error(lua, "addUser not implemented");
    return { };
}

ValuePack Computer::isRobot(lua_State* lua)
{
    return { false };
}

ValuePack Computer::tmpAddress(lua_State* lua)
{
    return { _tmp_address };
}

ValuePack Computer::freeMemory(lua_State* lua)
{
    return { std::numeric_limits<double>::max() };
}

ValuePack Computer::totalMemory(lua_State* lua)
{
    luaL_error(lua, "totalMemory not implemented");
    return { };
}

ValuePack Computer::energy(lua_State* lua)
{
    luaL_error(lua, "energy not implemented");
    return { };
}

ValuePack Computer::maxEnergy(lua_State* lua)
{
    luaL_error(lua, "maxEnergy not implemented");
    return { };
}

