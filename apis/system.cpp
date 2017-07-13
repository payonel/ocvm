#include "system.h"
#include "model/log.h"
#include "apis/unicode.h"

// statics
double SystemApi::_timeout = 5;
bool SystemApi::_gc = false;
bool SystemApi::_bytecode = false;
string SystemApi::_fonts;
string SystemApi::_bios;
string SystemApi::_machine;
int SystemApi::_max_connections = 4;

SystemApi::SystemApi() : LuaProxy("system")
{
    cadd("allowGC", &SystemApi::allowGC);
    cadd("timeout", &SystemApi::timeout);
    cadd("allowBytecode", &SystemApi::allowBytecode);
}

SystemApi* SystemApi::get()
{
    static SystemApi it;
    return &it;
}

int SystemApi::allowGC(lua_State* lua)
{
    return ValuePack::ret(lua, get()->_gc);
}

int SystemApi::timeout(lua_State* lua)
{
    return ValuePack::ret(lua, get()->_timeout);
}

int SystemApi::allowBytecode(lua_State* lua)
{
    return ValuePack::ret(lua, get()->_bytecode);
}

////////
void SystemApi::configure(const Value& settings)
{
    _timeout = settings.get("timeout").Or(_timeout).toNumber();
    _bytecode = settings.get("allowBytecode").Or(_bytecode).toBool();
    _gc = settings.get("allowGC").Or(_gc).toBool();
    _fonts = settings.get("fonts").Or("").toString();
    _bios = settings.get("bios").Or("").toString();
    _machine = settings.get("machine").Or("").toString();
    _max_connections = settings.get("maxTcpConnections").Or(_max_connections).toNumber();

    UnicodeApi::configure(_fonts);
}

string SystemApi::fonts_path()
{
    return _fonts;
}
string SystemApi::bios_path()
{
    return _bios;
}
string SystemApi::machine_path()
{
    return _machine;
}
int SystemApi::max_connections()
{
    return _max_connections;
}
