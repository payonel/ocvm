#include "component.h"
#include "model/log.h"
#include "model/client.h"

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
    _config = &config;
    this->name(config.get(ConfigIndex::Type).toString());
    _client = client;

    Value& vaddr = config.get(ConfigIndex::Address);
    if (vaddr.type() != "string" || vaddr.toString().empty())
    {
        vaddr = _address;
    }
    else
    {
        _address =  vaddr.toString();
    }

    return onInitialize();
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

    static bool rand_initialized = false;
    if (!rand_initialized)
    {
        srand(time(nullptr));
        rand_initialized = true;
    }

    for (auto len : sets)
    {
        if (!result.empty())
            result += "-";
        for (int index = 0; index < len; index++)
        {
            stringstream ss;
            ss << std::hex;
            ss << int(rand() % 256);
            string pair = ss.str();
            if (pair.size() == 1)
                pair = "0" + pair;
            result += pair;
        }
    }

    return result;
}

Client* Component::client() const
{
    return _client;
}

const Value& Component::config() const
{
    return *static_cast<const Value*>(_config);
}

void Component::update(int key, const Value& value)
{
    _config->set(key, value);
}
