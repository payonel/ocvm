#pragma once
#include "luaproxy.h"

class UnicodeApi : public LuaProxy
{
public:
    static UnicodeApi* get();

    ValuePack sub(const ValuePack& args);
private:
    UnicodeApi();
};
