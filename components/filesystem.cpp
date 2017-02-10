#include "filesystem.h"
#include "host.h"
#include "log.h"

Filesystem::Filesystem(const string& type, const Value& init, Host* host) :
    Component(type, init, host)
{
    // mkdir in host env path and filesystem address
    this->host()->mkdir(address());

    add("open", &Filesystem::open);
}

string Filesystem::path() const
{
    return host()->envPath() + "/" + address();
}

ValuePack Filesystem::open(const ValuePack& args)
{
    string path = Value::check(args, 0, "string").toString();
    Value vmode = Value::check(args, 1, "string", "nil");
    string mode = vmode ? vmode.toString() : "r";

    luaL_error(args.state, "todo");
}
