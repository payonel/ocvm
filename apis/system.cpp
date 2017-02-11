#include "system.h"
#include "log.h"

SystemApi::SystemApi() : LuaProxy("system")
{
    lout << "TODO hook api to configurations, not just components\n";
    lout << "this is needed, for example, for system to have a configured timeout\n";

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
    return ValuePack({_timeout});
}

ValuePack SystemApi::allowBytecode(const ValuePack& args)
{
    return ValuePack();
}

////////
void SystemApi::setTimeout(double t)
{
    _timeout = t;
}
