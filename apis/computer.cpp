#include "computer.h"

ComputerApi::ComputerApi() : LuaProxy("computer")
{
}

ComputerApi* ComputerApi::get()
{
    static ComputerApi it;
    return &it;
}
