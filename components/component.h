#pragma once
#include <string>
#include <vector>
#include <map>
#include "value.h"
#include "luaproxy.h"

class Component;
typedef ValuePack (Component::*ComponentMethod)(const ValuePack& args);

class Host;

class Component : public LuaProxy
{
public:
    Component(Value& config, Host* host);
    virtual ~Component() {}
    string type() const;
    string address() const;
    int slot() const;

    static string make_address();
protected:
    Host* host() const;
private:
    string _address;
    int _slot;
    Host* _host;
    
    ValuePack get_address(const ValuePack&);
    ValuePack get_type(const ValuePack&);
};
