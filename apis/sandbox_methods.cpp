#include "sandbox_methods.h"
#include "log.h"
#include "luaenv.h"

SandboxMethods::SandboxMethods(Client* pClient) :
    LuaProxy(""),
    _client(pClient)
{
}
