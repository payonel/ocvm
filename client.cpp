#include "client.h"
#include "host.h"
#include "components/component.h"
#include "config.h"
#include "log.h"
#include "luaenv.h"
#include <lua.hpp>

#include <string>
#include <functional>

using std::endl;
using std::string;
using std::vector;
using std::tuple;
using std::make_tuple;

Client::Client(Host* host) : LuaProxy("component"), _host(host)
{
    _config = new Config();

    add("list", &Client::component_list);
    add("invoke", &Client::component_invoke);
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
            lout << "bad config key, not string: " << pair.first.type() << std::endl;
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
    string filter = Value::select(args, 0).toString();
    bool exact = Value::select(args, 1).toBool();
    
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

    return ValuePack();
}

ValuePack Client::component_invoke(const ValuePack& args)
{
    // ValuePack component_invoke(const std::string& address, const std::string& methodName, const ValuePack& args);
    string address = Value::select(args, 0).toString();
    string methodName = Value::select(args, 1).toString();
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
