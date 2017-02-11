#pragma once
#include "luaproxy.h"

class SystemApi : public LuaProxy
{
public:
    static SystemApi* get();

    ValuePack timeout(const ValuePack& args);
    ValuePack allowGC(const ValuePack& args);
    ValuePack allowBytecode(const ValuePack& args);

    void configure(const Value& settings);
private:
    SystemApi();

    double _timeout = 5;
    bool _gc = false;
    bool _bytecode = false;
};
