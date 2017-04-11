#include "sandbox_methods.h"
#include "model/log.h"

SandboxMethods::SandboxMethods(Client* pClient) :
    LuaProxy(""),
    _client(pClient)
{
}
