#pragma once
#include "luaproxy.h"

class SystemApi : public LuaProxy
{
public:
    static SystemApi* get();

    ValuePack timeout(const ValuePack& args);
    ValuePack allowGC(const ValuePack& args);
    ValuePack allowBytecode(const ValuePack& args);

    void setTimeout(double t);
    void setAllowBytecode(bool enable);
    void setAllowGC(bool enable);
private:
    SystemApi();

    double _timeout = 5;
    bool _gc = false;
    bool _bytecode = false;
};
