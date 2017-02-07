#pragma once
#include <string>
#include <vector>
#include <map>
#include <lua.hpp>
#include "value.h"

class LuaProxy;
typedef ValuePack (LuaProxy::*ProxyMethod)(const ValuePack& args);
typedef std::tuple<std::string, lua_CFunction> LuaMethod;

class LuaProxy
{
public:
    LuaProxy(const std::string& name);
    virtual ~LuaProxy();

    const std::string& name() const;
    std::vector<LuaMethod> methods() const;
    ValuePack invoke(const std::string& methodName, const ValuePack& args);
protected:
    void add(const std::string& methodName, ProxyMethod method);
    template <typename Derived>
    void add(const std::string& methodName, ValuePack (Derived::*derivedMethod)(const ValuePack&))
    {
        add(methodName, static_cast<ProxyMethod>(derivedMethod));
    }
private:
    std::map<std::string, ProxyMethod> _methods;
    std::string _name;
};
