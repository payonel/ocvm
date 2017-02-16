#pragma once
#include "luaproxy.h"

class GlobalMethods : public LuaProxy
{
public:
    static GlobalMethods* get();
    int print(lua_State* lua);
    int error(lua_State* lua);
private:
    GlobalMethods();
};
