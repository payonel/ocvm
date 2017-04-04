#pragma once

#include "luaproxy.h"

class UserData
{
public:
    virtual vector<string> methods() const { return {}; }
    virtual void dispose() {}
};

class UserDataApi : public LuaProxy
{
public:
    static UserDataApi* get();
    static int methods(lua_State* lua);
    static int dispose(lua_State* lua);
private:
    UserDataApi();
};
