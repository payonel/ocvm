#include "computer.h"
#include "log.h"
#include "client.h"

#include <lua.hpp>
#include <iostream>
#include <thread>
#include <chrono>
using namespace std::chrono;

inline double now()
{
    return duration_cast<duration<double>>(system_clock::now().time_since_epoch()).count();
}

Computer::Computer()
{
    _start_time = now();

    add("address", &Computer::get_address);

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
    add("realTime", &Computer::realTime);
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

void Computer::setTmpAddress(const string& addr)
{
    _tmp_address = addr;
}

void Computer::pushSignal(const ValuePack& pack)
{
    //trace(nullptr, true);
    _signals.push(pack);
}

double Computer::trace(lua_State* coState, bool bForce)
{
    double thenow = now();
    if (_nexttrace < thenow || bForce)
    {
        _nexttrace = thenow + .5; // trace frequency
        if (!coState)
        {
            lua_Debug ar;
            lua_getstack(_state, 1, &ar);
            for (int n = 1; n < 10 && !coState; n++)
            {
                const char* cstrVarName = lua_getlocal(_state, &ar, n);
                if (!cstrVarName)
                {
                    break; // could not find coState
                }
                string varname = cstrVarName;
                if (varname == "co")
                {
                    coState = lua_tothread(_state, -1);
                }
                lua_pop(_state, 1);
            }
        }
        if (coState)
        {
            //std::cerr << Value::stack(coState) << endl;
        }
    }
    return thenow;
}

int Computer::realTime(lua_State* lua)
{
    return ValuePack::ret(lua, trace(lua));
}

void Computer::injectCustomLua()
{
    lua_getglobal(_state, name().c_str()); // +1

    //stringstream ss;
    //ss << _start_time;
    //string startTimeText = ss.str();
    //string code = "return " + startTimeText + " + os.clock()";
    //luaL_loadstring(_state, code.c_str()); // +1
    //lua_setfield(_state, -2, "realTime"); // -1

    lua_getglobal(_state, "os"); // +1
    lua_pushstring(_state, "clock"); //+1
    lua_gettable(_state, -2); // push clock on stack, pop key name, +1-1
    lua_remove(_state, -2); // pop os, -1
    lua_setfield(_state, -2, "uptime"); // computer.uptime = os.clock, pops clock, -1

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
    pushSignal(ValuePack::pack(lua));
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
    return ValuePack::ret(lua, false);
}

int Computer::tmpAddress(lua_State* lua)
{
    return ValuePack::ret(lua, _tmp_address);
}

int Computer::freeMemory(lua_State* lua)
{
    return ValuePack::ret(lua, std::numeric_limits<double>::max());
}

int Computer::totalMemory(lua_State* lua)
{
    return ValuePack::ret(lua, std::numeric_limits<double>::max());
}

int Computer::energy(lua_State* lua)
{
    return ValuePack::ret(lua, std::numeric_limits<double>::max());
}

int Computer::maxEnergy(lua_State* lua)
{
    return ValuePack::ret(lua, std::numeric_limits<double>::max());
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
        else if (_standby > now()) // return true without resume to return to the framer update
        {
            trace();
            return true;
        }
    }

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
            int type_id = lua_type(_state, 1);
            switch (type_id)
            {
                case LUA_TFUNCTION:
                    lua_pcall(_state, top - 1, LUA_MULTRET, 0);
                    top = lua_gettop(_state);
                    return resume(top);
                break;
                case LUA_TNUMBER:
                    _standby = std::max(0.0, lua_tonumber(_state, 1)) + now();
                break;
                case LUA_TBOOLEAN:
                    if (lua_toboolean(_state, 1)) // reboot
                    {
                    }
                    else // shutdown
                    {
                    }
                    // shutdown or reboot
                    return false;
                break;
                default:
                    lout << "unsupported yield: " << lua_typename(_state, 1) << endl;
                    return false;
                break;
            }
        }
        lua_settop(_state, 0);
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
    bool bCreated = false; // no need to set if exists

    if (!bGlobalMethods)
    {
        // check if it exists already
        lua_getglobal(_state, libname.c_str());
        if (lua_type(_state, -1) == LUA_TNIL)
        {
            bCreated = true;
            lua_pop(_state, 1);
            lua_newtable(_state); // create lib tbl
        }
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

    if (!bGlobalMethods && bCreated)
    {
        lua_setglobal(_state, libname.c_str()); // _G[libname] = tbl, pops tbl
    }

    return true;
}

int Computer::get_address(lua_State* lua)
{
    return ValuePack::ret(lua, address());
}
