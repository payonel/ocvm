#pragma once
#include "luaproxy.h"

class OSApi : public LuaProxy
{
public:
    static OSApi* get();
private:
    OSApi();
};
