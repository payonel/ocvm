#include "userdata.h"

UserDataApi::UserDataApi() : LuaProxy("userdata")
{
    cadd("methods", &UserDataApi::methods);
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
    Value vdata = Value::check(lua, 1, "userdata");
    void* ptr = vdata.toPointer();
    UserData* pData = reinterpret_cast<UserData*>(ptr);

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
    for (const auto& method_name : data->methods())
    {
        methods_table.insert(method_name);
    }

    return ValuePack::ret(lua, methods_table);
}

int UserDataApi::dispose(lua_State* lua)
{
    UserData* data = check(lua);
    data->dispose();
    return 0;
}

int UserDataApi::apply(lua_State* lua)
{
    return luaL_error(lua, "userdata.apply is not implemented");
}

int UserDataApi::unapply(lua_State* lua)
{
    return luaL_error(lua, "userdata.unapply is not implemented");
}

int UserDataApi::call(lua_State* lua)
{
    return luaL_error(lua, "userdata.call is not implemented");
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
    return luaL_error(lua, "userdata.doc is not implemented");
}

UserDataApi* UserDataApi::get()
{
    static UserDataApi api;
    return &api;
}

