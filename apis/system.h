#pragma once
#include "luaproxy.h"

class SystemApi : public LuaProxy
{
public:
    static SystemApi* get();

    ValuePack allowGC(const ValuePack& args);
    ValuePack timeout(const ValuePack& args);
    ValuePack allowBytecode(const ValuePack& args);

    void setTimeout(double t);
private:
    SystemApi();

    double _timeout = 5;
};
