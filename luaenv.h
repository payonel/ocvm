#pragma once
#include <string>
#include <vector>
#include <tuple>

struct lua_State;
typedef int (*LuaCallback)(lua_State*);

typedef std::tuple<std::string, LuaCallback> LuaMethod;

class LuaEnv
{
public:
    LuaEnv();
    ~LuaEnv();
    bool run();
    bool load(const std::string& machinePath);
    bool newlib(const std::string& libname, std::vector<LuaMethod> callbacks);
    void close();
private:
    lua_State* _state;
};
