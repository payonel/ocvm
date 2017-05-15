#include "sandbox_methods.h"
#include "model/log.h"

SandboxMethods::SandboxMethods(Client* pClient) :
    LuaProxy(""),
    _client(pClient)
{
    add("add_component", &SandboxMethods::add_component);
    add("remove_component", &SandboxMethods::remove_component);
}

int SandboxMethods::add_component(lua_State* lua)
{
    return 0;
}

int SandboxMethods::remove_component(lua_State* lua)
{
    return 0;
}

