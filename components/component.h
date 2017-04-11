#pragma once
#include <string>
#include <vector>
#include <map>
#include "model/value.h"
#include "model/luaproxy.h"

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
    enum ConfigIndex
    {
        Type = 1,
        Address,
        Next
    };

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
    virtual bool onInitialize() = 0;
    Client* client() const;
    const Value& config() const;
    void update(int key, const Value& value);
private:
    string _address;
    int _slot;
    Client* _client;
    Value* _config;
};
