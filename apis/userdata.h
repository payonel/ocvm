#pragma once

#include "model/luaproxy.h"
#include <functional>

class UserData : public LuaProxy
{
public:
  UserData();
  virtual ~UserData() = default;
  virtual void dispose() = 0;
private:
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

class UserDataAllocator
{
public:
  UserDataAllocator(lua_State* lua)
      : _lua(lua)
  {
  }

  UserData* operator()(size_t n) const
  {
    return reinterpret_cast<UserData*>(lua_newuserdata(_lua, n));
  }

private:
  lua_State* _lua;
};
