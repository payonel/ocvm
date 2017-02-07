#include "luaenv.h"
#include "log.h"
#include "luaproxy.h"
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

bool LuaEnv::newlib(LuaProxy* proxy)
{
    string libname = proxy->name();
    vector<LuaMethod> methods = proxy->methods();

    lua_newtable(_state); // create lib tbl

    for (const auto& tup : methods)
    {
        const string& name = std::get<0>(tup);
        lua_CFunction pf = std::get<1>(tup);

        lua_newtable(_state); // the method!
        lua_pushstring(_state, name.c_str());
        lua_setfield(_state, -2, "name");

        lua_newtable(_state); // mt
        lua_pushcfunction(_state, pf);
        lua_setfield(_state, -2, "__call"); // pops the function

        lua_pushlightuserdata(_state, proxy);
        lua_setfield(_state, -2, "instance"); // pops pinstance

        lua_setmetatable(_state, -2); // pops mt, to udata

        lua_setfield(_state, -2, name.c_str()); // tbl[name] = udata, pops udata
    }

    lua_setglobal(_state, libname.c_str()); // _G[libname] = tbl, pops tbl

    return true;
}
