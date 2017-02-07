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
    Component(const std::string& type, const Value& init);
    virtual ~Component() {}
    std::string type() const;
    std::string address() const;

    static std::string make_address();
protected:
private:
    std::string _type;
    std::string _address;
};
