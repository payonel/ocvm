#include "computer.h"
#include "model/log.h"
#include "model/client.h"
#include "filesystem.h"

#include <lua.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
using namespace std::chrono;

inline double now()
{
    return duration_cast<duration<double>>(system_clock::now().time_since_epoch()).count();
}

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
    add("realTime", &Computer::realTime);
    add("uptime", &Computer::uptime);
    add("isRunning", &Computer::isRunning);
}

Computer::~Computer()
{
    close();
}

static void* alloc_handler(void* ud, void* ptr, size_t osize, size_t nsize)
{
    Computer* pc = reinterpret_cast<Computer*>(ud);
    return pc->alloc(ptr, osize, nsize);
}

void* Computer::alloc(void* ptr, size_t osize, size_t nsize)
{
    osize = ptr ? osize : 0; // do not use osize if ptr is null
    if (nsize < osize) // drop
    {
        size_t drop = osize - nsize;
        if (_memory_used < drop) // !!
        {
            lerr << "critical emulator bug, memory used drop larger than used\n";
            drop = _memory_used;
        }
        _memory_used -= drop;
        // lprof << "memory:-" << drop << endl;
    }
    else // rise
    {
        size_t to_alloc = nsize - osize;
        size_t request_memory_use = _memory_used + to_alloc;
        if (_baseline_initialized)
        {
            // memory may drop below baseline
            if (request_memory_use > _baseline)
            {
                size_t machine_used = request_memory_use - _baseline;
                if (machine_used > _total_memory)
                {
                    lout << "vm out of memory\n";
                    return nullptr;
                }
            }
            // lprof << "memory:" << to_alloc << endl;
        }
        _memory_used = request_memory_use;
    }

    if (nsize == 0)
    {
        free(ptr);
        return 0;
    }
    return realloc(ptr, nsize);
}

bool Computer::onInitialize()
{
    this->client()->computer(this);

    {
        auto total_mem_setting = config().get(ConfigIndex::TotalMemory);
        if (total_mem_setting)
        {
            _total_memory = static_cast<size_t>(total_mem_setting.toNumber());
        }
        else
        {
            _total_memory = std::numeric_limits<size_t>::max();
        }
    }

    _state = lua_newstate(&alloc_handler, this);
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
        _nexttrace = thenow + 1; // trace frequency
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
            //string stack = Value::stack(coState);
            //lout << "stack: memory[" << _memory_used << "]" << stack << endl;
            // lprof << "stack\n";
        }
    }
    return thenow;
}

int Computer::realTime(lua_State* lua)
{
    return ValuePack::ret(lua, trace(lua));
}

int Computer::uptime(lua_State* lua)
{
    return ValuePack::ret(lua, trace() - _start_time);
}

void Computer::injectCustomLua()
{
    lua_getglobal(_state, name().c_str()); // +1

    string code = "return [=====[" + address() + "]=====]";
    luaL_loadstring(_state, code.c_str()); // (function) +1
    lua_setfield(_state, -2, "address"); // computer.address = function, pops loadstring -1

    lua_getglobal(_state, "os"); // +1

    lua_pushstring(_state, "time"); // +1
    lua_gettable(_state, -2); // push time on stack, pop key name, +1-1
    lua_setfield(_state, -2, "_time"); // os._time = os.time, pops time, -1
    // now override time function
    luaL_loadstring
    (_state,
        "local t = ...              \n"
        "if type(t) == 'table' then \n"
        "    t.isdst = false        \n"
        "end                        \n"
        "return os._time(...)       \n"
    ); // +1
    lua_setfield(_state, -2, "time"); // os.time = function, pops function, -1

    lua_pushstring(_state, "date"); // +1
    lua_gettable(_state, -2); // push date on stack, pop key name, +1-1
    lua_setfield(_state, -2, "_date"); // os._date = os.date, pops time, -1
    // now override date function (to be exception safe)
    luaL_loadstring
    (_state,
        "if select('#', ...) == 0 then                      \n"
        "    return os._date()                              \n"
        "end                                                \n"
        "local function check(c)                            \n"
        "    return pcall(os._date, c) and c or ''          \n"
        "end                                                \n"
        "local arg = ...                                    \n"
        "if type(arg) == 'nil' then                         \n"
        "    arg = '%d/%m/%y %H:%M:%S'                      \n"
        "elseif type(arg) == 'string' then                  \n"
        "    local t = {...}                                \n"
        "    if arg:find('\\0') then                        \n"
        "        return arg:gsub('([^\\0]+)', function(c)   \n"
        "                return os.date(c,                  \n"
        "                   select(2, table.unpack(t)))     \n"
        "            end)                                   \n"
        "    end                                            \n"
        "    arg = arg:gsub('%%[GUVWZguz]','')              \n"
        "             :gsub('^!', '')                       \n"
        "             :gsub('%%?.', check)                  \n"
        "else                                               \n"
        "    arg = check(arg)                               \n"
        "end                                                \n"
        "local ok, ret = pcall(os._date,                    \n"
        "    select(1, arg, select(2, ...)))                \n"
        "if ok and type(ret) == 'table' then                \n"
        "    ret.isdst = nil                                \n"
        "end                                                \n"
        "return ok and ret or ''                            \n"
    ); // +1
    lua_setfield(_state, -2, "date"); // os.date = function, pops function, -1

    lua_pop(_state, 1); // pop os, -1

    //stringstream ss;
    //ss << _start_time;
    //string startTimeText = ss.str();
    //string code = "return " + startTimeText + " + os.clock()";
    //luaL_loadstring(_state, code.c_str()); // +1
    //lua_setfield(_state, -2, "realTime"); // -1

    // lua_getglobal(_state, "os"); // +1
    // lua_pushstring(_state, "clock"); //+1
    // lua_gettable(_state, -2); // push clock on stack, pop key name, +1-1
    // lua_remove(_state, -2); // pop os, -1   
    // lua_setfield(_state, -2, "uptime"); // computer.uptime = os.clock, pops clock, -1

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
    return ValuePack::ret(lua, Value::table()
        .insert(Value::table().insert("build").insert("builder").set("n", 2))
        .insert(Value::table().insert("dig").insert("dig").set("n", 2))
        .insert(Value::table().insert("base64").insert("data").set("n", 2))
        .insert(Value::table().insert("deflate").insert("data").set("n", 2))
        .insert(Value::table().insert("gpg").insert("data").set("n", 2))
        .insert(Value::table().insert("inflate").insert("data").set("n", 2))
        .insert(Value::table().insert("md5sum").insert("data").set("n", 2))
        .insert(Value::table().insert("sha256sum").insert("data").set("n", 2))
        .insert(Value::table().insert("refuel").insert("generator").set("n", 2))
        .insert(Value::table().insert("irc").insert("irc").set("n", 2))
        .insert(Value::table().insert("maze").insert("maze").set("n", 2))
        .insert(Value::table().insert("arp").insert("network").set("n", 2))
        .insert(Value::table().insert("ifconfig").insert("network").set("n", 2))
        .insert(Value::table().insert("ping").insert("network").set("n", 2))
        .insert(Value::table().insert("route").insert("network").set("n", 2))
        .insert(Value::table().insert("opl-flash").insert("openloader").set("n", 2))
        .insert(Value::table().insert("oppm").insert("oppm").set("n", 2))
    );
}

int Computer::pushSignal(lua_State* lua)
{
    pushSignal(ValuePack::pack(lua));
    return 0;
}

bool Computer::postInit()
{
    for (auto* pc : client()->components("filesystem", true))
    {
        auto pfs = dynamic_cast<Filesystem*>(pc);
        if (pfs && pfs->isTmpfs())
        {
            setTmpAddress(pc->address());
            return true;
        }
    }

    lout << "missing tmpfs\n";
    return false;
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
    size_t free_memory = 0;
    if (_memory_used <= _total_memory)
        free_memory = _total_memory - _memory_used;

    return ValuePack::ret(lua, free_memory);
}

int Computer::totalMemory(lua_State* lua)
{
    return ValuePack::ret(lua, _total_memory);
}

int Computer::energy(lua_State* lua)
{
    return ValuePack::ret(lua, std::numeric_limits<double>::max());
}

int Computer::maxEnergy(lua_State* lua)
{
    return ValuePack::ret(lua, std::numeric_limits<double>::max());
}

RunState Computer::update()
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
            // std::this_thread::yield();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return RunState::Continue;
        }
    }

    RunState result = resume(nargs);
    if (bFirstTimeRun)
    {
        _baseline = _memory_used;
        _baseline_initialized = true;
        lout << "lua env baseline\n";
    }
    return result;
}

RunState Computer::resume(int nargs)
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
        int thread_index = 0;
        int size = thread.len();
        // [1] should be the thread itself, [2] is the pcall return
        for (int index = 0; index < size; index++)
        {
            if (thread.get(index).type_id() == LUA_TTHREAD)
            {
                thread_index = index;
                break;
            }
        }
        if (thread_index == 0)
        {
            lout << "kernal panic, failed to detect thread in stack\n";
        }
        else
        {
            Value yield_value = thread.get(thread_index + 1);
            if (!yield_value)
            {
                lout << "kernel panic: " << thread.toString() << endl;
            }
            else
            {
                lout << "lua env SHUTDOWN\n";
            }
        }

        return RunState::Halt;
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
                    lua_pcall(_state, -1, LUA_MULTRET, 0);
                    top = lua_gettop(_state);
                    return resume(top);
                break;
                case LUA_TNUMBER:
                    _standby = std::max(0.0, lua_tonumber(_state, 1)) + now();
                break;
                case LUA_TBOOLEAN:
                    if (lua_toboolean(_state, 1)) // reboot
                    {
                        return RunState::Reboot;
                    }
                    else // shutdown
                    {
                        return RunState::Halt;
                    }
                    // shutdown or reboot
                break;
                default:
                    lout << "unsupported yield: " << lua_typename(_state, 1) << endl;
                    return RunState::Halt;
                break;
            }
        }
        lua_settop(_state, 0);
    }
    else
    {
        lout << "host crash: ";
        lout << lua_tostring(_state, -1) << "\n";
        lout << "machine status: " << Value(_state).serialize() << endl;
        return RunState::Halt;
    }

    return RunState::Continue;
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

int Computer::isRunning(lua_State* lua)
{
    return ValuePack::ret(lua, true);
}
