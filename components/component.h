#pragma once
#include <string>
#include <vector>
#include <map>
#include "value.h"
#include "luaproxy.h"

class Component;
typedef ValuePack (Component::*ComponentMethod)(const ValuePack& args);

class Component : public LuaProxy
{
public:
    Component(const string& type, const Value& init);
    virtual ~Component() {}
    string type() const;
    string address() const;

    static string make_address();
protected:
private:
    string _type;
    string _address;
};
