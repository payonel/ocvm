#include "luaenv.h"
#include "log.h"
#include <iostream>
#include <lua.hpp>

LuaEnv::LuaEnv()
{
    _state = luaL_newstate();
    luaL_openlibs(_state);
}

LuaEnv::~LuaEnv()
{
    close();
}

bool LuaEnv::run()
{
    lout << "lua env resume\n";
    return false;
}

bool LuaEnv::load(const std::string& machinePath)
{
    if (luaL_loadfile(_state, machinePath.c_str()))
    {
        lout << "failed to load string\n";
        lout << lua_tostring(_state, -1) << "\n";
        lua_pop(_state, 1);
        return false;
    }

    lout << "machine loaded\n";

    if (lua_pcall(_state, 0, LUA_MULTRET, 0))
    {
        lout << "pcall failure\n";
        lout << lua_tostring(_state, -1) << "\n";
        lua_pop(_state, 1);
        return false;
    }

    lout << "machine state returned\n";

    return true;
}

void LuaEnv::close()
{
    if (_state)
    {
        lua_close(_state);
        _state = nullptr;
        lout << "lua env closed\n";
    }
}