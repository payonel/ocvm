#pragma once
#include "luaproxy.h"

class GlobalMethods : public LuaProxy
{
public:
    static GlobalMethods* get();
    static int print(lua_State* lua);
    static int error(lua_State* lua);
private:
    GlobalMethods();
};
