#pragma once
#include "model/luaproxy.h"

class SystemApi : public LuaProxy
{
public:
    static SystemApi* get();

    static int timeout(lua_State* lua);
    static int allowGC(lua_State* lua);
    static int allowBytecode(lua_State* lua);

    static void configure(const Value& settings);
private:
    SystemApi();

    static double _timeout;
    static bool _gc;
    static bool _bytecode;
};
