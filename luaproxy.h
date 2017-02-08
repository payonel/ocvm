#pragma once
#include <string>
#include <vector>
#include <map>
#include <lua.hpp>
#include "value.h"

using std::tuple;

class LuaProxy;
typedef ValuePack (LuaProxy::*ProxyMethod)(const ValuePack& args);
typedef tuple<string, lua_CFunction> LuaMethod;

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
    ValuePack invoke(const string& methodName, const ValuePack& args);
protected:
    void add(const string& methodName, ProxyMethod method);
    template <typename Derived>
    void add(const string& methodName, ValuePack (Derived::*derivedMethod)(const ValuePack&))
    {
        add(methodName, static_cast<ProxyMethod>(derivedMethod));
    }
private:
    map<string, ProxyMethod> _methods;
    string _name;
};
