#pragma once
#include <string>
#include <vector>
#include <map>
#include "value.h"
#include "luaproxy.h"

class Component;
typedef ValuePack (Component::*ComponentMethod)(const ValuePack& args);

class Client;

class Component : public LuaProxy
{
public:
    Component();
    bool initialize(Client* client, Value& config);
    virtual ~Component() {}
    string type() const;
    string address() const;
    int slot() const;

    static string make_address();
protected:
    virtual bool onInitialize(Value& config) = 0;
    Client* client() const;
private:
    string _address;
    int _slot;
    Client* _client;
    
    ValuePack get_address(const ValuePack&);
    ValuePack get_type(const ValuePack&);
};
