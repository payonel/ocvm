#pragma once
#include "luaproxy.h"

class GlobalMethods : public LuaProxy
{
public:
    static GlobalMethods* get();
    ValuePack print(lua_State* lua);
    ValuePack error(lua_State* lua);
private:
    GlobalMethods();
};
