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
    log << "lua env resume\n";
    return false;
}

bool LuaEnv::load(const std::string& machinePath)
{
    if (luaL_loadfile(_state, machinePath.c_str()))
    {
        log << "failed to load string\n";
        log << lua_tostring(_state, -1) << "\n";
        lua_pop(_state, 1);
        return false;
    }

    log << "machine loaded\n";

    if (lua_pcall(_state, 0, LUA_MULTRET, 0))
    {
        log << "pcall failure\n";
        log << lua_tostring(_state, -1) << "\n";
        lua_pop(_state, 1);
        return false;
    }

    log << "machine state returned\n";

    return true;
}

void LuaEnv::close()
{
    if (_state)
    {
        lua_close(_state);
        _state = nullptr;
        log << "lua env closed\n";
    }
}