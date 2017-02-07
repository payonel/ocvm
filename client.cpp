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

int _a(lua_State* lua)
{
    Value udata_call = Value::make(lua, 1);
    string filter = Value::make(lua, 2).toString();
    bool exact = Value::make(lua, 3).toBool();

    const Value& mt = udata_call.metatable();
    if (mt)
    {
        const Value& inst = mt.get("instance");
        if (inst.type() == "userdata")
        {
            void* p = inst.toPointer();
            Client* pc = static_cast<Client*>(p);
            vector<Component*> comps = pc->component_list(filter, exact);
        }
    }

    return 0;
}

int _b(lua_State*)
{
    lout << "from _b\n";
    return 1;
}

Client::Client(Host* host) : LuaProxy("component"), _host(host)
{
    _config = new Config();

    add("list", &_a);
    add("invoke", &_b);
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

    auto pc_vec = component_list("gpu");
    if (!pc_vec.empty())
    {
        auto* pc = pc_vec.at(0);
        component_invoke(pc->address(), "setResolution", Value::pack(50, 16));
    }

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
