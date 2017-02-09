#include "global_methods.h"
#include "log.h"
#include "luaenv.h"

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

ValuePack GlobalMethods::print(const ValuePack& args)
{
    bool bFirst = true;
    for (const auto& arg : args)
    {
        string separator = bFirst ? "[--vm--] " : "\t";
        lout << separator << arg.toString();
        bFirst = false;
    }

    lout << endl;
    return ValuePack();
}

ValuePack GlobalMethods::error(const ValuePack& args)
{
    lout << "[--vm--] [ERROR] " << Value::select(args, 0).toString() << endl;
    lout << LuaEnv::stack(args.state) << endl;
    return ValuePack();
}

