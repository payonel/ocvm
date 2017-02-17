#include "sandbox_methods.h"
#include "log.h"

SandboxMethods::SandboxMethods(Client* pClient) :
    LuaProxy(""),
    _client(pClient)
{
}
