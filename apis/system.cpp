#include "system.h"

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

ValuePack SystemApi::allowGC(const ValuePack& args)
{
    return ValuePack();
}

ValuePack SystemApi::timeout(const ValuePack& args)
{
    return ValuePack();
}

ValuePack SystemApi::allowBytecode(const ValuePack& args)
{
    return ValuePack();
}
