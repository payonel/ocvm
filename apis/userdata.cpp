#include "userdata.h"

UserData::UserData()
    : LuaProxy("unnamed")
{
}

UserDataApi::UserDataApi()
    : LuaProxy("userdata")
{
  cadd("methods", &UserDataApi::methods);
  cadd("invoke", &UserDataApi::invoke);
  cadd("dispose", &UserDataApi::dispose);
  cadd("apply", &UserDataApi::apply);
  cadd("unapply", &UserDataApi::unapply);
  cadd("call", &UserDataApi::call);
  cadd("save", &UserDataApi::save);
  cadd("load", &UserDataApi::load);
  cadd("doc", &UserDataApi::doc);
}

static UserData* check(lua_State* lua)
{
  UserData* pData = reinterpret_cast<UserData*>(Value::checkArg<void*>(lua, 1));

  if (!pData)
  {
    luaL_error(lua, "bad pointer to user data");
  }

  return pData;
}

int UserDataApi::methods(lua_State* lua)
{
  UserData* data = check(lua);
  Value methods_table = Value::table();
  for (const LuaMethod& info : data->methods())
  {
    methods_table.set(std::get<0>(info), true);
  }

  return ValuePack::ret(lua, methods_table);
}

int UserDataApi::invoke(lua_State* lua)
{
  UserData* data = check(lua);
  // the pointer is probably safe from gc..probably
  lua_remove(lua, 1);
  string methodName = Value::checkArg<string>(lua, 1);
  lua_remove(lua, 1);

  int stacked = data->invoke(methodName, lua);
  lua_pushboolean(lua, true);
  lua_insert(lua, 1);
  return stacked + 1;
}

int UserDataApi::dispose(lua_State* lua)
{
  UserData* data = check(lua);
  data->dispose();
  data->~UserData();
  return 0;
}

int UserDataApi::apply(lua_State* lua)
{
  return ValuePack::ret(lua, true);
}

int UserDataApi::unapply(lua_State* lua)
{
  return ValuePack::ret(lua, true);
}

int UserDataApi::call(lua_State* lua)
{
  return UserDataApi::invoke(lua);
}

int UserDataApi::save(lua_State* lua)
{
  return luaL_error(lua, "userdata.save is not implemented");
}

int UserDataApi::load(lua_State* lua)
{
  return luaL_error(lua, "userdata.load is not implemented");
}

int UserDataApi::doc(lua_State* lua)
{
  return ValuePack::ret(lua, Value::nil);
}

UserDataApi* UserDataApi::get()
{
  static UserDataApi api;
  return &api;
}
