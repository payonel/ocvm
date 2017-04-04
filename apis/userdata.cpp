#include "userdata.h"

UserDataApi::UserDataApi() : LuaProxy("userdata")
{
    cadd("methods", &UserDataApi::methods);
    cadd("dispose", &UserDataApi::dispose);
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

UserDataApi* UserDataApi::get()
{
    static UserDataApi api;
    return &api;
}
