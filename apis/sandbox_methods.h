#pragma once
#include "luaproxy.h"

class Client;

class SandboxMethods : public LuaProxy
{
public:
    SandboxMethods(Client* pClient);
private:
    Client* _client;
};
