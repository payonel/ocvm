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

// set meta table
    // lua_newtable(_state);
    // lua_pushnumber(_state, 7);
    // lua_setfield(_state, -2, "datum");
    // lua_setmetatable(_state, -2); // setmetatable(new_table_on_top_of_stack,{datum=7})

bool LuaEnv::newlib(const std::string& libname, std::vector<LuaMethod> callbacks, std::vector<LightField> lfields)
{
    lua_newtable(_state);
    for (size_t i = 0; i < callbacks.size(); i++)
    {
        const auto& tup = callbacks.at(i);
        lua_pushcfunction(_state, std::get<1>(tup));
        lua_setfield(_state, -2, std::get<0>(tup).c_str());
    }
    for (size_t i = 0; i < lfields.size(); i++)
    {
        const auto& tup = lfields.at(i);
        lua_pushlightuserdata(_state, std::get<1>(tup));
        lua_setfield(_state, -2, std::get<0>(tup).c_str());
    }
    lua_setglobal(_state, libname.c_str());

    return true;
}

