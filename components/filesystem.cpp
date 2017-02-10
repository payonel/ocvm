#include "filesystem.h"
#include "log.h"

Filesystem::Filesystem(const string& type, const Value& init, Host* host) :
    Component(type, init, host)
{
}
