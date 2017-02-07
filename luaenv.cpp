#include "luaenv.h"
#include "log.h"
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
#include <lua.hpp>

using std::string;
using std::vector;

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

bool LuaEnv::load(const string& machinePath)
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

bool LuaEnv::newlib(const string& libname, vector<LuaMethod> callbacks, void* pinstance)
{
    lua_newtable(_state); // create tbl
    for (const auto& tup : callbacks)
    {
        const string& name = std::get<0>(tup);
        LuaCallback pf = std::get<1>(tup);

        lua_newuserdata(_state, sizeof(LuaInstanceMethod));

        lua_newtable(_state); // mt
        lua_pushcfunction(_state, pf);
        lua_setfield(_state, -2, "__call"); // pops the function

        lua_pushlightuserdata(_state, pinstance);
        lua_setfield(_state, -2, "instance"); // pops pinstance

        lua_setmetatable(_state, -2); // pops mt, to udata

        lua_setfield(_state, -2, name.c_str()); // tbl[name] = udata, pops udata
    }

    lua_setglobal(_state, libname.c_str()); // _G[libname] = tbl, pops tbl

    return true;
}

