#pragma once
#include <string>

struct lua_State;

class LuaEnv
{
public:
    LuaEnv();
    bool run();
    bool load(const std::string& machinePath);
    void close();
private:
    lua_State* _state;
};
