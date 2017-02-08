#pragma once
#include <string>
#include <vector>
#include <tuple>
#include <lua.hpp>

using std::string;

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
    bool load(const string& machinePath);
    bool newlib(LuaProxy* proxy);
    void close();
private:
    lua_State* _state = nullptr;
    lua_State* _machine = nullptr;
};
