#pragma once
#include "luaproxy.h"

class UnicodeApi : public LuaProxy
{
public:
    static UnicodeApi* get();
private:
    UnicodeApi();
};
