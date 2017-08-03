#pragma once

#include "model/luaproxy.h"

class UserData : public LuaProxy
{
public:
    UserData();
    virtual ~UserData() {}
    virtual void dispose() {}
};

class UserDataApi : public LuaProxy
{
public:
    static UserDataApi* get();
    static int methods(lua_State* lua);
    static int invoke(lua_State* lua);
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
