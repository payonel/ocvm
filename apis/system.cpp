#include "system.h"
#include "log.h"

SystemApi::SystemApi() : LuaProxy("system")
{
    add("allowGC", &SystemApi::allowGC);
    add("timeout", &SystemApi::timeout);
    add("allowBytecode", &SystemApi::allowBytecode);
}

SystemApi* SystemApi::get()
{
    static SystemApi it;
    return &it;
}

int SystemApi::allowGC(lua_State* lua)
{
    Value(get()->_gc).push(lua);
    return 1;
}

int SystemApi::timeout(lua_State* lua)
{
    Value(get()->_timeout).push(lua);
    return 1;
}

int SystemApi::allowBytecode(lua_State* lua)
{
    Value(get()->_bytecode).push(lua);
    return 1;
}

////////
void SystemApi::configure(const Value& settings)
{
    _timeout = settings.get("timeout").Or(_timeout).toNumber();
    _bytecode = settings.get("allowBytecode").Or(_bytecode).toBool();
    _gc = settings.get("allowGC").Or(_gc).toBool();
}
