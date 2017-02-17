#include "system.h"
#include "log.h"

SystemApi::SystemApi() : LuaProxy("system")
{
    cadd("allowGC", &SystemApi::allowGC);
    cadd("timeout", &SystemApi::timeout);
    cadd("allowBytecode", &SystemApi::allowBytecode);
}

SystemApi* SystemApi::get()
{
    static SystemApi it;
    return &it;
}

int SystemApi::allowGC(lua_State* lua)
{
    return ValuePack::push(lua, get()->_gc);
}

int SystemApi::timeout(lua_State* lua)
{
    return ValuePack::push(lua, get()->_timeout);
}

int SystemApi::allowBytecode(lua_State* lua)
{
    return ValuePack::push(lua, get()->_bytecode);
}

////////
void SystemApi::configure(const Value& settings)
{
    _timeout = settings.get("timeout").Or(_timeout).toNumber();
    _bytecode = settings.get("allowBytecode").Or(_bytecode).toBool();
    _gc = settings.get("allowGC").Or(_gc).toBool();
}
