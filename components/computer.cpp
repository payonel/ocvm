#include "computer.h"
#include "model/log.h"
#include "model/client.h"
#include "model/host.h"
#include "filesystem.h"
#include "apis/system.h"
#include "drivers/fs_utils.h"

#include <lua.hpp>
#include <iostream>
#include <thread>
#include <chrono>
#include <cstring>
using std::chrono::duration_cast;
using std::chrono::duration;
using std::chrono::system_clock;
using Logging::lout;

const float memory_scale = 1;

bool Computer::s_registered = Host::registerComponentType<Computer>("computer");

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

    add("crash", &Computer::crash);
    add("print", &Computer::print);
}

void Computer::stackLog(const string& stack_log)
{
    _prof.open(stack_log);
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

string get_stacktrace(lua_State* lua)
{
    lua_State* coState = nullptr;
    lua_Debug ar;
    lua_getstack(lua, 1, &ar);
    for (int n = 1; n < 10 && !coState; n++)
    {
        const char* cstrVarName = lua_getlocal(lua, &ar, n);
        if (!cstrVarName)
        {
            break; // could not find coState
        }
        if (cstrVarName[0] == 'c' &&
            cstrVarName[1] == 'o' &&
            cstrVarName[2] == '\0')
        {
            coState = lua_tothread(lua, -1);
        }
        lua_pop(lua, 1);
    }

    if (coState)
        return Value::stack(coState);
    return "";
}

void* Computer::alloc(void* ptr, size_t osize, size_t nsize)
{
    osize = ptr ? osize : 0; // do not use osize if ptr is null
    if (nsize > osize) // rise
    {
        size_t to_alloc = nsize - osize;
        size_t free_mem = freeMemory();
        if (to_alloc > free_mem)
        {
            return nullptr;
        }
    }

    string stacktrace;
    if (_prof.is_open())
    {
        if (_baseline_initialized && nsize != 0)
        {
            stacktrace = get_stacktrace(_state);
        }

        if (ptr && (nsize == 0 || nsize != osize))
        {
            _prof.release(ptr);
        }
    }

    void* ret_ptr = nullptr;
    if (nsize != 0)
        ret_ptr = realloc(ptr, nsize);

    if (_prof.is_open() && _baseline_initialized && ret_ptr)
        _prof.trace(stacktrace, ret_ptr, nsize);

    if (nsize == 0)
    {
        free(ptr);
        return nullptr;
    }
    return ret_ptr;
}

bool Computer::onInitialize()
{
    this->client()->computer(this);

    {
        auto total_mem_setting = config().get(ConfigIndex::TotalMemory);
        if (total_mem_setting)
        {
            _total_memory = static_cast<size_t>(total_mem_setting.toNumber() * memory_scale);
        }
        else
        {
            _total_memory = std::numeric_limits<size_t>::max();
        }
    }

    _state = lua_newstate(&alloc_handler, this);
    luaL_openlibs(_state); // needed for common globals
    newlib(this);

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

int Computer::realTime(lua_State* lua)
{
    return ValuePack::ret(lua, now());
}

int Computer::uptime(lua_State* lua)
{
    return ValuePack::ret(lua, now() - _start_time);
}

struct load_reader_data
{
    string code;
    bool used = false;
};

const char* load_reader(lua_State* lua, void* pvReader, size_t* pSizeOut)
{
    load_reader_data* pReader = static_cast<load_reader_data*>(pvReader);
    if (pReader->used)
    {
        *pSizeOut = 0;
        return nullptr;
    }
    pReader->used = true;
    *pSizeOut = pReader->code.size();
    return pReader->code.data();
}

static void inject_address(lua_State* lua, const string& address)
{
    lua_getglobal(lua, "computer"); // push computer, +1
    string code = "return [=====[" + address + "]=====]";
    luaL_loadstring(lua, code.c_str()); // (function) +1
    lua_setfield(lua, -2, "address"); // computer.address = function, pops loadstring -1
    lua_pop(lua, 1); // pop computer, -1
}

static void inject_time(lua_State* lua)
{
    lua_getglobal(lua, "os"); // push os, +1
    lua_pushstring(lua, "time"); // +1
    lua_gettable(lua, -2); // push time on stack, pop key name, +1-1
    lua_setfield(lua, -2, "_time"); // os._time = os.time, pops time, -1
    // now override time function
    luaL_loadstring
    (lua,
        "local t = ...              \n"
        "if type(t) == 'table' then \n"
        "    t.isdst = false        \n"
        "end                        \n"
        "return os._time(...)       \n"
    ); // +1
    lua_setfield(lua, -2, "time"); // os.time = function, pops function, -1
    lua_pop(lua, 1); // pop os, -1
}

static void inject_date(lua_State* lua)
{
    lua_getglobal(lua, "os"); // push os, +1
    lua_pushstring(lua, "date"); // +1
    lua_gettable(lua, -2); // push date on stack, pop key name, +1-1
    lua_setfield(lua, -2, "_date"); // os._date = os.date, pops time, -1
    // now override date function (to be exception safe)
    luaL_loadstring
    (lua,
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
    lua_setfield(lua, -2, "date"); // os.date = function, pops function, -1
    lua_pop(lua, 1); // pop os, -1
}

static void inject_xp_load(lua_State* lua)
{
    // backup the original load
    lua_getglobal(lua, "load"); // +1
    lua_setglobal(lua, "_load"); // -1

    load_reader_data data
    {
        "load = _load                                           \n"
        "local func, msg = load(...)                            \n"
        "if not func then                                       \n"
        "    return func, msg                                   \n"
        "end                                                    \n"
        "return function(...)                                   \n"
        "    return select(2, assert(xpcall(func, computer.crash, ...))) \n"
        "end                                                    \n"
    };

    lua_load(lua, load_reader, &data, "_load", "t"); // function, +1

    lua_setglobal(lua, "load"); // pop function, -1
}

static void inject_print(lua_State* lua)
{
    lua_getglobal(lua, "computer"); // push computer, +1

    lua_pushstring(lua, "print"); // +1
    lua_gettable(lua, -2); // push print on stack, pop key name, +1-1

    lua_setglobal(lua, "print"); // print = computer.print, -1
    lua_pop(lua, 1); // pop computer, -1
}

void Computer::injectCustomLua()
{
    inject_address(_state, address());
    inject_time(_state);
    inject_date(_state);
    inject_xp_load(_state);
    inject_print(_state);
}

int Computer::setArchitecture(lua_State* lua)
{
    string arch = Value::checkArg<string>(lua, 1);
    if (LUA_VERSION != arch)
    {
        return luaL_error(lua, "cannot change architecture");
    }

    return 0;
}

int Computer::getArchitecture(lua_State* lua)
{
    return ValuePack::ret(lua, LUA_VERSION);
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
        }
    }

    injectCustomLua();

    _machine = lua_newthread(_state);
    if (_machine == nullptr)
    {
        client()->append_crash("failed to create machine state");
        return false;
    }

    lout << "machine thread created\n";

    string machine_path = client()->host()->machinePath();

    std::string data;
    if (!fs_utils::read(machine_path, &data))
    {
        client()->append_crash("failed to read machine file [" + machine_path + "]");
        return false;
    }
    if (luaL_loadbuffer(_state, data.c_str(), data.size(), "machine.lua"))
    {
        client()->append_crash("failed to load machine [" + machine_path + "]");
        client()->append_crash(lua_tostring(_state, -1));
        lua_pop(_state, 1);
        return false;
    }
    lout << "machine function loaded\n";

    return true;
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
    if (_tmp_address.empty())
        return ValuePack::ret(lua, false, "no tmpfs");
    return ValuePack::ret(lua, _tmp_address);
}

int Computer::freeMemory(lua_State* lua)
{
    return ValuePack::ret(lua, freeMemory() / memory_scale);
}

int Computer::totalMemory(lua_State* lua)
{
    return ValuePack::ret(lua, _total_memory / memory_scale);
}

int Computer::energy(lua_State* lua)
{
    return ValuePack::ret(lua, std::numeric_limits<LUA_NUMBER>::max());
}

int Computer::maxEnergy(lua_State* lua)
{
    return ValuePack::ret(lua, std::numeric_limits<LUA_NUMBER>::max());
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
            // std::this_thread::yield();
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
            return RunState::Continue;
        }
    }

    RunState result = resume(nargs);
    if (bFirstTimeRun)
    {
        // we seem to allocate a bit more above real oc
        _baseline = memoryUsedRaw() + 91227;
        _baseline_initialized = true;
        lout << "lua env baseline: " << _baseline << endl;
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
                string report = thread.get(thread.len()).toString();
                client()->append_crash("kernel panic: " + report);
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
            lua_Number timeout_limit = 0;
            switch (type_id)
            {
                case LUA_TFUNCTION:
                    lua_pcall(_state, -1, LUA_MULTRET, 0);
                    top = lua_gettop(_state);
                    return resume(top);
                break;
                case LUA_TNIL:
                    timeout_limit = std::numeric_limits<lua_Number>::max();
                case LUA_TNUMBER:
                    mark_gc();
                    _standby = std::max(timeout_limit, lua_tonumber(_state, 1)) + now();
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
                    lout << "unsupported yield: " << lua_typename(_state, type_id) << endl;
                    return RunState::Halt;
                break;
            }
        }
        lua_settop(_state, 0);
    }
    else
    {
        lout << "vm crash: ";
        lout << lua_tostring(_state, -1) << "\n";
        lout << "machine stack: " << Value::stack(_machine) << endl;
        lout << "machine status: " << Value(_machine).serialize() << endl;
        return RunState::Halt;
    }

    return RunState::Continue;
}

void Computer::close()
{
    if (_state)
    {
        lua_close(_state);
        _state = nullptr;
        lout << "lua env closed\n";
    }

    _prof.flush();

    lout << "computer peek memory: " << _peek_memory << endl;
    _peek_memory = 0;
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

void Computer::mark_gc()
{
    _gc_ticks++;
    if (_gc_ticks >= 10)
    {
        lua_gc(_state, LUA_GCCOLLECT, 0); // data, last param, not used for collect
        _gc_ticks = 0;
    }
}

size_t Computer::memoryUsedRaw()
{
    if (_state == nullptr)
        return 0;

    return lua_gc(_state, LUA_GCCOUNT, 0) * 1024 + lua_gc(_state, LUA_GCCOUNTB, 0);
}

size_t Computer::memoryUsedVM()
{
    if (!_baseline_initialized)
        return 0;

    size_t raw = memoryUsedRaw();
    if (raw <= _baseline)
        return 0;

    size_t used = raw - _baseline;
    _peek_memory = std::max(_peek_memory, used);
    return used;
}

size_t Computer::freeMemory()
{
    if (!_baseline_initialized)
        return std::numeric_limits<size_t>::max();

    size_t vm_used = memoryUsedVM();
    if (vm_used >= _total_memory)
        return 0;

    return _total_memory - vm_used;
}

int Computer::crash(lua_State* lua)
{
    client()->append_crash(lua_tostring(lua, -1));
    luaL_traceback(lua, lua, nullptr, 1);
    if (lua_type(lua, -1) == LUA_TSTRING)
    {
        client()->append_crash(lua_tostring(lua, -1));
    }
    lua_pop(lua, 1);
    return lua_gettop(lua);
}

int Computer::print(lua_State* lua)
{
    bool bFirst = true;
    int top = lua_gettop(lua);
    string msg;
    for (int i = 1; i <= top; i++)
    {
        string separator = bFirst ? "[--vm--] " : "\t";
        msg += separator;
        msg += Value(lua, i).toString();
        bFirst = false;
    }

    msg += "\n";

    lout << msg;
    return ValuePack::ret(lua, true);
}
