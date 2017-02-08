#include "luaenv.h"
#include "log.h"
#include "luaproxy.h"
#include <iostream>
#include <string>
#include <tuple>
#include <vector>
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

int run_caller(lua_State* lua)
{
    lout << "in lua callback\n";
    return 0;
}

bool LuaEnv::run()
{
    lout << "lua env resume: " << lua_gettop(_state) - 2 << endl;
    int status_id = lua_resume(_state, _machine, 0);
    if (status_id == LUA_OK)
    {
        lout << "lua env SHUTDOWN\n";
        return false;
    }
    else if (status_id == LUA_YIELD)
    {
        lout << "lua env yielded\n";
        return true;
    }
    else
    {
        lout << "machine crash: ";
        lout << lua_tostring(_state, -1) << "\n";
        lout << "machine thread" << Value(_state).toString() << endl;
        lout << "main thread" << Value(_machine).toString() << endl;
        return false;
    }

    return true;
}

bool LuaEnv::load(const string& machinePath)
{
    _machine = lua_newthread(_state);
    lout << "machine thread created\n";

    if (luaL_loadfile(_state, machinePath.c_str()))
    {
        lout << "failed to load machine\n";
        lout << lua_tostring(_state, -1) << "\n";
        lua_pop(_state, 1);
        return false;
    }
    lout << "machine function loaded\n";

    return _machine;
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

    bool bGlobalMethods = libname.empty();

    if (!bGlobalMethods)
    {
        lua_newtable(_state); // create lib tbl
    }

    for (const auto& tup : methods)
    {
        const string& name = std::get<0>(tup);
        lua_CFunction pf = std::get<1>(tup);

        lua_newtable(_state);
        lua_pushstring(_state, name.c_str());
        lua_setfield(_state, -2, "name");
        lua_pushlightuserdata(_state, proxy);
        lua_setfield(_state, -2, "instance");

        lua_pushcclosure(_state, pf, 1);

        if (bGlobalMethods)
        {
            lua_setglobal(_state, name.c_str());
        }
        else
        {
            lua_setfield(_state, -2, name.c_str()); // tbl[name] = udata, pops udata
        }
    }

    if (!bGlobalMethods)
    {
        lua_setglobal(_state, libname.c_str()); // _G[libname] = tbl, pops tbl
    }

    return true;
}
