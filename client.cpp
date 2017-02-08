#include "client.h"
#include "host.h"
#include "components/component.h"
#include "config.h"
#include "log.h"
#include "luaenv.h"
#include <lua.hpp>

#include "apis/os.h"
#include "apis/computer.h"
#include "apis/global_methods.h"

#include <string>
#include <functional>

using std::endl;
using std::make_tuple;

Client::Client(Host* host) : LuaProxy("component"), _host(host)
{
    _config = new Config();

    add("list", &Client::component_list);
    add("invoke", &Client::component_invoke);
    add("methods", &Client::component_methods);
}

Client::~Client()
{
    close();
}

bool Client::load(LuaEnv* lua)
{
    if (!_config->load(_host->envPath(), "client"))
    {
        lout << "failed to load client config\n";
        return false;
    }

    // load components from config
    for (auto pair : _config->pairs())
    {
        if (pair.first.type() != "string")
        {
            lout << "bad config key, not string: " << pair.first.type() << endl;
            return false;
        }
        else
        {
            string key = pair.first.toString();
            lout << key << ": ";
            Component* pc = _host->create(key, pair.second);
            if (pc)
            {
                lout << "created\n";
                _components.push_back(pc);
            }
            else
            {
                lout << "failed\n";
                return false;
            }
        }
    }
    lout << "components loaded: " << _components.size() << "\n";

    return loadLuaComponentApi(lua);
}

bool Client::loadLuaComponentApi(LuaEnv* lua)
{
    lua->newlib(this);
    lua->newlib(OSApi::get());
    lua->newlib(ComputerApi::get());
    lua->newlib(GlobalMethods::get());
    return true;
}

void Client::close()
{
    if (_config)
    {
        _config->save();
        delete _config;
        _config = nullptr;
    }

    for (auto pc : _components)
        delete pc;

    _components.clear();
}

ValuePack Client::component_list(const ValuePack& args)
{
    const Value& vfilter = Value::check(args, 0, "string", "nil");
    string filter = vfilter ? vfilter.toString() : "";

    bool exact = Value::check(args, 1, "boolean", "nil").toBool();

    ValuePack pack;
    Value result = Value::table();

    for (auto* pc : _components)
    {
        string type = pc->type();
        if (type.find(filter) == 0)
        {
            if (!exact || type == filter)
            {
                result.set(pc->address(), pc->type());
            }
        }
    }

    pack.push_back(result);
    return pack;
}

ValuePack Client::component_invoke(const ValuePack& args)
{
    string address = Value::check(args, 0, "string").toString();
    string methodName = Value::check(args, 1, "string").toString();

    ValuePack pack;

    for (auto* pc : _components)
    {
        if (pc->address() == address)
        {
            return pc->invoke(methodName, pack);
        }
    }

    return ValuePack();
}

ValuePack Client::component_methods(const ValuePack& args)
{
    string address = Value::check(args, 0, "string").toString();

    ValuePack result;
    for (auto* pc : _components)
    {
        if (pc->address() == address)
        {
            Value mpack = Value::table();
            for (const auto& luaMethod : pc->methods())
            {
                mpack.set(std::get<0>(luaMethod), true);
            }
            result.push_back(mpack);
            break;
        }
    }

    if (result.empty()) // addr not found
    {
        result.push_back(Value::nil);
        result.push_back("no such component");
    }

    return result;
}
