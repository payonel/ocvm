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

    stringstream ss;
    ss << _start_time;
    string code = "return computer.realTime() - " + ss.str() + " ";
    luaL_loadstring(lua, code.c_str());
    lua_setfield(lua, -2, "uptime");
}

int Computer::setArchitecture(lua_State* lua)
{
    luaL_error(lua, "setArchitecture not implemented");
    return 0;
}

int Computer::getArchitecture(lua_State* lua)
{
    luaL_error(lua, "getArchitecture not implemented");
    return 0;
}

int Computer::getArchitectures(lua_State* lua)
{
    luaL_error(lua, "getArchitectures not implemented");
    return 0;
}

int Computer::beep(lua_State* lua)
{
    lout << "\a" << "BEEP\n";
    return 0;
}

int Computer::getDeviceInfo(lua_State* lua)
{
    luaL_error(lua, "getDeviceInfo not implemented");
    return 0;
}

int Computer::getProgramLocations(lua_State* lua)
{
    luaL_error(lua, "getProgramLocations not implemented");
    return 0;
}

int Computer::pushSignal(lua_State* lua)
{
    return 0;
}

int Computer::removeUser(lua_State* lua)
{
    luaL_error(lua, "removeUser not implemented");
    return 0;
}

int Computer::addUser(lua_State* lua)
{
    luaL_error(lua, "addUser not implemented");
    return 0;
}

int Computer::isRobot(lua_State* lua)
{
    return ValuePack::push(lua, false);
}

int Computer::tmpAddress(lua_State* lua)
{
    return ValuePack::push(lua, _tmp_address);
}

int Computer::freeMemory(lua_State* lua)
{
    return ValuePack::push(lua, std::numeric_limits<double>::max());
}

int Computer::totalMemory(lua_State* lua)
{
    luaL_error(lua, "totalMemory not implemented");
    return 0;
}

int Computer::energy(lua_State* lua)
{
    luaL_error(lua, "energy not implemented");
    return 0;
}

int Computer::maxEnergy(lua_State* lua)
{
    luaL_error(lua, "maxEnergy not implemented");
    return 0;
}

