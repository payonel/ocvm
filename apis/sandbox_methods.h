#pragma once
#include "model/luaproxy.h"

class Client;

class SandboxMethods : public LuaProxy
{
public:
    SandboxMethods(Client* pClient);

    static int add_component(lua_State* lua);
    static int remove_component(lua_State* lua);
private:
    Client* _client;
};
