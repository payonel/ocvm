#include "computer.h"
#include "log.h"
#include "client.h"

#include <lua.hpp>
#include <iostream>
#include <thread>
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

Computer::~Computer()
{
    close();
}

bool Computer::onInitialize(Value& config)
{
    this->client()->computer(this);

    _state = luaL_newstate();
    luaL_openlibs(_state); // needed for common globals
    newlib(this);
    injectCustomLua();

    return _state != nullptr;
}

int64_t Computer::now()
{
    return duration_cast<seconds>(system_clock::now().time_since_epoch()).count();
}

void Computer::setTmpAddress(const string& addr)
{
    _tmp_address = addr;
}

void Computer::injectCustomLua()
{
    lua_getglobal(_state, name().c_str()); // +1

    lua_getglobal(_state, "os"); // +1
    lua_pushstring(_state, "time"); // push key name, +1
    lua_gettable(_state, -2); // push time on stack, pop key name, +1-1
    lua_remove(_state, -2); // pop os, -1
    lua_setfield(_state, -2, "realTime"); // computer.realTime = time, pops time, -1

    stringstream ss;
    ss << _start_time;
    string code = "return computer.realTime() - " + ss.str() + " ";
    luaL_loadstring(_state, code.c_str()); // +1
    lua_setfield(_state, -2, "uptime"); // -1

    lua_pop(_state, 1); // -1
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
    _signals.push(ValuePack::pack(lua));
    return 0;
}

int Computer::removeUser(lua_State* lua)
{
    return 0;
}

int Computer::addUser(lua_State* lua)
{
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
    return ValuePack::push(lua, std::numeric_limits<double>::max());
}

int Computer::energy(lua_State* lua)
{
    return ValuePack::push(lua, std::numeric_limits<double>::max());
}

int Computer::maxEnergy(lua_State* lua)
{
    return ValuePack::push(lua, std::numeric_limits<double>::max());
}

bool Computer::run()
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
    int env_status = lua_status(_state);
    bool bFirstTimeRun = env_status == LUA_OK; // FIRST time run, all other resumes come from yield
    int nargs = 0;
    if (!bFirstTimeRun)
    {
        // kbcode signals?
        // modem message signals?
        if (!_signals.empty())
        {
            nargs = _signals.front().push(_state);
            _signals.pop();
        }
        else if (_standby < now()) // return true without resume to return to the framer update
        {
            return true;
        }
    }
    //sleep
    // else if timeout, nargs = 0
    // else don't resume
    _standby = now();
    bool result = resume(nargs);
    if (bFirstTimeRun)
    {
        // create memory baseline
    }
    return result;
}

bool Computer::resume(int nargs)
{
    //lout << "lua env resume: " << nargs << endl;
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
        //lout << "lua env yielded\n";
        int top = lua_gettop(_state);
        if (top > 0)
        {
            Value result(_state, -1);
            switch (result.type_id())
            {
                case LUA_TFUNCTION:
                    lua_settop(_state, 1);
                    lua_pcall(_state, 0, LUA_MULTRET, 0);
                    // the returns, if any, are on the stack
                    // should be given to the machine to resume
                    top = lua_gettop(_state);
                    return resume(top);
                break;
                case LUA_TNUMBER:
                    _standby = std::max(0.0, result.toNumber()) + now();
                break;
                case LUA_TBOOLEAN:
                    // shutdown or reboot
                    return false;
                break;
                default:
                    lout << "unsupported yield: " << result.type() << endl;
                    return false;
                break;
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

bool Computer::load(const string& machinePath)
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

void Computer::close()
{
    if (_state)
    {
        lua_close(_state);
        _state = nullptr;
        lout << "lua env closed\n";
    }
}

bool Computer::newlib(LuaProxy* proxy)
{
    string libname = proxy->name();

    bool bGlobalMethods = libname.empty();

    if (!bGlobalMethods)
    {
        lua_newtable(_state); // create lib tbl
    }

    for (const auto& tup : proxy->methods())
    {
        const string& name = std::get<0>(tup);
        bool isDirect = std::get<1>(tup);
        lua_CFunction pf = std::get<2>(tup);

        if (isDirect)
        {
            lua_pushcfunction(_state, pf);
        }
        else
        {
            lua_newtable(_state);
            lua_pushstring(_state, name.c_str());
            lua_setfield(_state, -2, "name");
            lua_pushlightuserdata(_state, proxy);
            lua_setfield(_state, -2, "instance");
            lua_pushcclosure(_state, pf, 1);
        }

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

