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

ValuePack SystemApi::allowGC(lua_State* lua)
{
    return ValuePack { _gc };
}

ValuePack SystemApi::timeout(lua_State* lua)
{
    return ValuePack { _timeout };
}

ValuePack SystemApi::allowBytecode(lua_State* lua)
{
    return ValuePack { _bytecode };
}

////////
void SystemApi::configure(const Value& settings)
{
    _timeout = settings.get("timeout").Or(_timeout).toNumber();
    _bytecode = settings.get("allowBytecode").Or(_bytecode).toBool();
    _gc = settings.get("allowGC").Or(_gc).toBool();
}
