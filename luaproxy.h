#pragma once
#include <string>
#include <vector>
#include <map>
#include <lua.hpp>
#include "value.h"

using std::tuple;

class LuaProxy;
typedef ValuePack (LuaProxy::*ProxyMethod)(lua_State* lua);
typedef tuple<string, bool, lua_CFunction> LuaMethod;

class LuaProxy
{
public:
    LuaProxy(const string& name);
    virtual ~LuaProxy();

    void operator=(LuaProxy) = delete;
    LuaProxy(const LuaProxy&) = delete;
    LuaProxy(LuaProxy&&) = delete;

    const string& name() const;
    vector<LuaMethod> methods() const;
    ValuePack invoke(const string& methodName, lua_State* lua);
    virtual void injectCustomLua(lua_State* lua) {}
protected:
    void name(const string& v);

    void add(const string& methodName, ProxyMethod method);
    template <typename Derived>
    void add(const string& methodName, ValuePack (Derived::*derivedMethod)(lua_State* lua))
    {
        add(methodName, static_cast<ProxyMethod>(derivedMethod));
    }
    void cadd(const string& methodName, lua_CFunction cfunction);
private:
    map<string, ProxyMethod> _methods;
    map<string, lua_CFunction> _cmethods; // for statics - faster dispatch
    string _name;
};
