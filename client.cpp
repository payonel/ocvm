#include "client.h"
#include "host.h"
#include "components/component.h"
#include "config.h"
#include "log.h"
#include "luaenv.h"
#include <lua.hpp>

#include "apis/os.h"
#include "apis/global_methods.h"
#include "apis/system.h"

#include <string>
#include <functional>

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
                lout << "failed! The host could not create a " << key << endl;
                return false;
            }
        }
    }
    lout << "components loaded: " << _components.size() << "\n";

    return loadLuaComponentApi(lua);
}

bool Client::loadLuaComponentApi(LuaEnv* lua)
{
    // computer required
    auto vec = components("computer", true);
    if (vec.size() != 1)
    {
        lout << "emulation requires exactly one computer component\n";
        return false;
    }
    auto* pc = vec.at(0);

    lua->newlib(pc); // computer component is also a global api
    lua->newlib(this);
    lua->newlib(OSApi::get());
    lua->newlib(GlobalMethods::get());
    lua->newlib(SystemApi::get());
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

vector<Component*> Client::components(string filter, bool exact) const
{
    vector<Component*> result;

    for (auto* pc : _components)
    {
        string type = pc->type();
        if (type.find(filter) == 0)
        {
            if (!exact || type == filter)
            {
                result.push_back(pc);
            }
        }
    }

    return result;
}

Component* Client::component(const string& address) const
{
    for (auto* pc : _components)
    {
        if (pc->address() == address)
        {
            return pc;
        }
    }
    return nullptr;
}

ValuePack Client::component_list(const ValuePack& args)
{
    const Value& vfilter = Value::check(args, 0, "string", "nil");
    string filter = vfilter ? vfilter.toString() : "";

    bool exact = Value::check(args, 1, "boolean", "nil").toBool();

    Value result = Value::table();
    auto matches = components(filter, exact);
    for (auto* pc : matches)
    {
        result.set(pc->address(), pc->type());
    }

    ValuePack pack(args.state);
    pack.push_back(result);
    return pack;
}

ValuePack Client::component_invoke(const ValuePack& args)
{
    // for logging, this is called via LuaProxy because all method calls are dispatched there first
    // LuaProxy::invoke has already logged much about this call, but is waiting to log the result
    // but, logging from here on out will look like a return value, so we add some indentation here
    lout << "\n_\n";
    string address = Value::check(args, 0, "string").toString();
    string methodName = Value::check(args, 1, "string").toString();

    ValuePack pack(args.state);
    ValuePack result;
    Component* pc = component(address);
    if (pc)
    {
        result = pc->invoke(methodName, pack);
        result.insert(result.begin(), true);
    }
    else
    {
        result.push_back(Value::nil);
        result.push_back("no such component");
    }

    lout << "_:";
    return result;
}

ValuePack Client::component_methods(const ValuePack& args)
{
    string address = Value::check(args, 0, "string").toString();

    ValuePack result(args.state);
    Component* pc = component(address);
    if (pc)
    {
        Value mpack = Value::table();
        Value info = Value::table();
        info.set("direct", true);
        for (const auto& luaMethod : pc->methods())
        {
            mpack.set(std::get<0>(luaMethod), info);
        }
        result.push_back(mpack);
    }
    else
    {
        result.push_back(Value::nil);
        result.push_back("no such component");
    }

    return result;
}

