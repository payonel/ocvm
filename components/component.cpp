#include "component.h"
#include "log.h"

#include <random>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

Component::Component(const string& type, const Value& init, Host* host) :
    LuaProxy(type),
    _type(type),
    _host(host)
{
    Value v;
    v = init.get(1);
    if (!v || v.type() != "string" || v.toString().empty())
        v = Value(make_address());
    _address = v.toString();
}

string Component::type() const
{
    return _type;
}

string Component::address() const
{
    return _address;
}

string Component::make_address()
{
    // -- e.g. 3c44c8a9-0613-46a2-ad33-97b6ba2e9d9a
    // -- 8-4-4-4-12 (halved sizes because bytes make hex pairs)
    vector<int> sets {4, 2, 2, 2, 6};
    string result = "";

    srand(time(nullptr));

    for (auto len : sets)
    {
        if (!result.empty())
            result += "-";
        for (int index = 0; index < len; index++)
        {
            stringstream ss;
            ss << std::hex;
            ss << int(rand() % 256);
            result += ss.str();
        }
    }

    return result;
}

Host* Component::host() const
{
    return _host;
}
