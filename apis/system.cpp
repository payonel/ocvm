#include "system.h"

SystemApi::SystemApi() : LuaProxy("system")
{
}

SystemApi* SystemApi::get()
{
    static SystemApi it;
    return &it;
}
