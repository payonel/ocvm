#pragma once
#include "luaproxy.h"

class GlobalMethods : public LuaProxy
{
public:
    static GlobalMethods* get();
    ValuePack print(const ValuePack& args);
    ValuePack error(const ValuePack& args);
    ValuePack loadfile(const ValuePack& args);
private:
    GlobalMethods();
};
