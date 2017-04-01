#include "global_methods.h"
#include "log.h"

GlobalMethods::GlobalMethods() : LuaProxy("")
{
    add("print", &GlobalMethods::print);
    add("error", &GlobalMethods::error);
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
    // string msg = Value(lua, 1).toString();
    // string stack = Value::stack(lua);
    // lout << "[--vm--] [ERROR] " << msg << endl;
    // lout << stack << endl;

    lua_settop(lua, 1);
    return lua_error(lua);
}
