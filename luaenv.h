#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <lua.hpp>

struct lua_State;
typedef int (*LuaCallback)(lua_State*);

struct LuaInstanceMethod
{
    void* instance;
    LuaCallback method;
};

class LuaProxy;

class LuaEnv
{
public:
    LuaEnv();
    ~LuaEnv();
    bool run();
    bool load(const std::string& machinePath);
    bool newlib(LuaProxy* proxy);
    void close();
private:
    lua_State* _state;
};
