#pragma once
#include <string>
#include <vector>
#include <map>
#include <lua.hpp>

// class Component;
// typedef ValuePack (Component::*ComponentMethod)(const ValuePack& args);

typedef std::tuple<std::string, lua_CFunction> LuaMethod;

class LuaProxy
{
public:
    LuaProxy(const std::string& name);
    virtual ~LuaProxy();

    const std::string& name() const;
    std::vector<LuaMethod> methods() const;
protected:
    void add(const std::string& methodName, lua_CFunction method);
private:
    std::map<std::string, lua_CFunction> _methods;
    std::string _name;
};
