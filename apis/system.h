#pragma once
#include "luaproxy.h"

class SystemApi : public LuaProxy
{
public:
    static SystemApi* get();

    static int timeout(lua_State* lua);
    static int allowGC(lua_State* lua);
    static int allowBytecode(lua_State* lua);

    void configure(const Value& settings);
private:
    SystemApi();

    double _timeout = 5;
    bool _gc = false;
    bool _bytecode = false;
};
