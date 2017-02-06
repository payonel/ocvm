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

Client::Client(Host* host) : _host(host)
{
    _config = new Config(host->envPath(), "client");
}

Client::~Client()
{
    close();
}

bool Client::load(LuaEnv* lua)
{
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

    auto pc_vec = component_list("gpu");
    if (!pc_vec.empty())
    {
        auto* pc = pc_vec.at(0);
        component_invoke(pc->address(), "setResolution", Value::pack(50, 16));
    }

    return loadLuaComponentApi(lua);
}

int _a(lua_State* lua)
{
    Value method_name = Value::make(lua, 0);
    Value method = Value::make(lua, -1);
    lout << "from _a\n";
    return 0;
}

int _b(lua_State*)
{
    lout << "from _b\n";
    return 1;
}

bool Client::loadLuaComponentApi(LuaEnv* lua)
{
    vector<LuaMethod> methods;
    methods.push_back(make_tuple("list", &_a));
    methods.push_back(make_tuple("invoke", &_b));

    vector<LightField> lfields;
    lfields.push_back(make_tuple("client", this));

    lua->newlib("component", methods, lfields);

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

vector<Component*> Client::component_list(const string& filter, bool exact)
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

ValuePack Client::component_invoke(const std::string& address, const string& methodName, const ValuePack& args)
{
    for (auto* pc : _components)
    {
        if (pc->address() == address)
        {
            return pc->invoke(methodName, args);
        }
    }

    return ValuePack();
}
