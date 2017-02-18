#include "component.h"
#include "log.h"

#include <random>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>

Component::Component() :
    LuaProxy("unnamed"),
    _address(make_address()),
    _slot(-1),
    _client(nullptr)
{
}

bool Component::initialize(Client* client, Value& config)
{
    this->name(config.get(1).toString());
    _client = client;

    Value& vaddr = config.get(2);
    if (vaddr.type() != "string" || vaddr.toString().empty())
    {
        vaddr = _address;
    }
    else
    {
        _address =  vaddr.toString();
    }

    return onInitialize(config);
}

string Component::type() const
{
    return LuaProxy::name();
}

string Component::address() const
{
    return _address;
}

int Component::slot() const
{
    return _slot;
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

Client* Component::client() const
{
    return _client;
}
