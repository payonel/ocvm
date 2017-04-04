#include "global_methods.h"
#include "log.h"

GlobalMethods::GlobalMethods() : LuaProxy("")
{
    cadd("print", &GlobalMethods::print);
    cadd("error", &GlobalMethods::error);
}

GlobalMethods* GlobalMethods::get()
{
    static GlobalMethods it;
    return &it;
}

int GlobalMethods::print(lua_State* lua)
{
    bool bFirst = true;
    int top = lua_gettop(lua);
    for (int i = 1; i <= top; i++)
    {
        string separator = bFirst ? "[--vm--] " : "\t";
        lout << separator << Value(lua, i).toString();
        bFirst = false;
    }

    lout << endl;
    return 0;
}

int GlobalMethods::error(lua_State* lua)
{
    // TODO this doesn't compile in 5.3 (luaL_optint not defined)
    // int level = luaL_optint(lua, 2, 1);
    // lua_settop(lua, 1);
    // if (lua_isstring(lua, 1) && level > 0)
    // {
    //     luaL_where(lua, level);
    //     lua_pushvalue(lua, 1);
    //     lua_concat(lua, 2);
    // }

    // string msg = Value(lua, 1).toString();
    // lout << "[--vm--] [ERROR] " << msg << endl;
    
    return lua_error(lua);
}
