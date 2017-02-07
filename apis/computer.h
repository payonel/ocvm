#pragma once

#include "luaproxy.h"

class ComputerApi : public LuaProxy
{
public:
    static ComputerApi* get();
private:
    ComputerApi();
};
