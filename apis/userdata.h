#pragma once

#include "luaproxy.h"

class UserData
{
public:
    virtual ~UserData() {}
    virtual vector<string> methods() const { return {}; }
    virtual void dispose() {}
};

class UserDataApi : public LuaProxy
{
public:
    static UserDataApi* get();
    static int methods(lua_State* lua);
    static int dispose(lua_State* lua);
    static int apply(lua_State* lua);
    static int unapply(lua_State* lua);
    static int call(lua_State* lua);
    static int save(lua_State* lua);
    static int load(lua_State* lua);
    static int doc(lua_State* lua);
private:
    UserDataApi();
};
