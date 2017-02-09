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
    /*
        Types of runs
        1. First time run
            main state is LUA_OK, else LUA_YIELD
            after resume, take memory baseline
        3. machine signals
            pop a signal off the queue and send it to the machine
        4. sleep timeout
            send 0 args to the machine
    */
    Value env(_state);
    bool bFirstTimeRun = env.status() == LUA_OK; // FIRST time run, all other resumes come from yield
    int nargs = 0;
    //machine signals
    // if signal, nargs = 1
    //sleep
    // else if timeout, nargs = 0
    // else don't resume
    bool result = resume(nargs);
    if (bFirstTimeRun)
    {
        // create memory baseline
    }
    return result;
}

bool LuaEnv::resume(int nargs)
{
    lout << "lua env resume: " << nargs << endl;
    int status_id = lua_resume(_state, _machine, nargs);
    /*
        Types of results
        1. OK
            The pcall in the host thread returned, meaning the machine shutdown or crashed
        2. YIELD
            The machine intentionally yielded and likely passed args back for processing
            a. function: direct target.invoke failed on a component
                we resume this function on the main thread, I don't know why
            b. number: time to sleep (decline to resume)
            c. bool: true:reboot, else shutdown
        3. CRASH/DEAD
            the host executor failed - e.g. machine.lua failed to compile(load)
    */
    if (status_id == LUA_OK)
    {
        Value thread(_state);
        if (!thread.get(2)) // [1] should be the thread itself, [2] is the pcall return
        {
            lout << "kernel panic: " << thread.get(3).toString() << endl;
        }
        else
            lout << "lua env SHUTDOWN\n";
        return false;
    }
    else if (status_id == LUA_YIELD)
    {
        lout << "lua env yielded\n";
        int top = lua_gettop(_state);
        if (top > 0)
        {
            Value result(_state, -1);
            if (result.type_id() == LUA_TFUNCTION)
            {
                lua_settop(_state, 1);
                lua_pcall(_state, 0, LUA_MULTRET, 0);
                // the returns, if any, are on the stack
                // should be given to the machine to resume
                top = lua_gettop(_state);
                return resume(top);
            }
        }
        return true;
    }
    else
    {
        lout << "host crash: ";
        lout << lua_tostring(_state, -1) << "\n";
        lout << "machine status: " << Value(_state).serialize() << endl;
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

string LuaEnv::stack(lua_State* state)
{
    luaL_traceback(state, state, NULL, 1);
    int top = lua_gettop(state);
    string stacktrace = string(lua_tostring(state, -1));
    lua_pop(state, 1);
    return stacktrace;
}
