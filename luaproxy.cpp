#include "luaproxy.h"

using std::vector;
using std::string;

LuaProxy::LuaProxy(const std::string& name) :
    _name(name)
{
}

LuaProxy::~LuaProxy()
{
}

const std::string& LuaProxy::name() const
{
    return _name;
}

vector<LuaMethod> LuaProxy::methods() const
{
    vector<LuaMethod> result;
    for (const auto& pair : _methods)
    {
        result.push_back(std::make_tuple(pair.first, pair.second));
    }
    return result;
}

void LuaProxy::add(const string& methodName, lua_CFunction method)
{
    _methods[methodName] = method;
}

