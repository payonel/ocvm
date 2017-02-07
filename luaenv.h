#pragma once
#include <string>
#include <vector>
#include <tuple>

struct lua_State;
typedef int (*LuaCallback)(lua_State*);

typedef std::tuple<std::string, LuaCallback> LuaMethod;
typedef std::tuple<std::string, void*> LightField;

struct LuaInstanceMethod
{
    void* instance;
    LuaCallback method;
};

class LuaEnv
{
public:
    LuaEnv();
    ~LuaEnv();
    bool run();
    bool load(const std::string& machinePath);
    bool newlib(const std::string& libname, 
        std::vector<LuaMethod> callbacks, 
        void* pinstance);
    void close();
private:
    lua_State* _state;
};
