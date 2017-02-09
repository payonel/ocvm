#pragma once
#include "luaproxy.h"

class SystemApi : public LuaProxy
{
public:
    static SystemApi* get();
private:
    SystemApi();
};
