#include "luaproxy.h"
#include "log.h"
#include <iostream>

LuaProxy::LuaProxy(const string& name) :
    _name(name)
{
}

LuaProxy::~LuaProxy()
{
}

const string& LuaProxy::name() const
{
    return _name;
}

void LuaProxy::name(const string& v)
{
    _name = v;
}

int lua_proxy_static_caller(lua_State* lua)
{
    lua_pushstring(lua, "instance");//+1
    lua_gettable(lua, lua_upvalueindex(1));//-1,+1
    auto _this = const_cast<void*>(lua_topointer(lua, -1));
    lua_pop(lua, 1);//-1

    if (!_this)
    {
        luaL_error(lua, "invalid callback, no instance pointer in closure");
        return 0;
    }

    lua_pushstring(lua, "name");//+1
    lua_gettable(lua, lua_upvalueindex(1));//-1,+1
    string methodName = lua_tostring(lua, -1);
    lua_pop(lua, 1);//-1

    LuaProxy* p = reinterpret_cast<LuaProxy*>(_this);
    return p->invoke(methodName, lua);
}

vector<LuaMethod> LuaProxy::methods() const
{
    vector<LuaMethod> result;
    for (const auto& pair : _methods)
    {
        result.push_back(std::make_tuple(pair.first, false, &lua_proxy_static_caller));
    }
    for (const auto& pair : _cmethods)
    {
        result.push_back(std::make_tuple(pair.first, true, pair.second));
    }
    return result;
}

void LuaProxy::add(const string& methodName, ProxyMethod method)
{
    _methods[methodName] = method;
}

void LuaProxy::cadd(const string& methodName, lua_CFunction cfunction)
{
    _cmethods[methodName] = cfunction;
}

int LuaProxy::invoke(const string& methodName, lua_State* lua)
{
    const auto& mit = _methods.find(methodName);
    if (mit == _methods.end())
    {
        lout << "no such method " << methodName << "\n";
        luaL_error(lua, "no such method: %s", methodName.c_str());
    }

    ProxyMethod pmethod = mit->second;
    return ((*this).*pmethod)(lua);
}
