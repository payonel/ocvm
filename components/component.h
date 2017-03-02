#pragma once
#include <string>
#include <vector>
#include <map>
#include "value.h"
#include "luaproxy.h"

class Component;
typedef ValuePack (Component::*ComponentMethod)(const ValuePack& args);

class Client;

enum class RunState
{
    Continue,
    Reboot,
    Halt
};

class Component : public LuaProxy
{
public:
    Component();
    bool initialize(Client* client, Value& config);
    virtual bool postInit() { return true; }
    virtual ~Component() {}
    string type() const;
    string address() const;
    int slot() const;

    virtual RunState update() { return RunState::Continue; }

    static string make_address();
protected:
    virtual bool onInitialize(Value& config) = 0;
    Client* client() const;
private:
    string _address;
    int _slot;
    Client* _client;
};
