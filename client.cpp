#include "client.h"
#include "host.h"
#include "components/component.h"
#include "components/filesystem.h"
#include "components/computer.h"

#include "config.h"
#include "log.h"
#include "luaenv.h"
#include "utils.h"
#include <lua.hpp>

#include "apis/os.h"
#include "apis/global_methods.h"
#include "apis/system.h"
#include "apis/unicode.h"

#include <string>
#include <functional>

using std::make_tuple;

Client::Client(Host* host, const string& env_path) : 
    LuaProxy("component"), 
    _env_path(env_path),
    _host(host)
{
    _config = new Config();

    add("list", &Client::component_list);
    add("invoke", &Client::component_invoke);
    add("methods", &Client::component_methods);
    add("type", &Client::component_type);
    add("slot", &Client::component_slot);

    // make the env path if it doesn't already exist
    utils::mkdir(_env_path);
}

Client::~Client()
{
    close();
}

bool Client::load(LuaEnv* lua)
{
    if (!_config->load(envPath(), "client"))
    {
        lout << "failed to load client config\n";
        return false;
    }

    if (!createComponents())
        return false;
    lout << "components loaded: " << _components.size() << "\n";

    if (!postInit())
        return false;
    lout << "components post initialized: " << _components.size() << "\n";

    return loadLuaComponentApi(lua);
}

bool Client::createComponents()
{
    // load components from config
    for (auto& pair : _config->pairs())
    {
        string section = pair.first.toString();
        if (section == "components")
        {
            for (auto& component_config_pair : pair.second.pairs())
            {
                auto& component_config = component_config_pair.second;
                string key = component_config.get(1).toString();
                lout << key << ": ";
                Component* pc = _host->create(key);
                if (!(pc && pc->initialize(this, component_config)))
                {
                    lout << "failed! The host could not create a " << key << endl;
                    return false;
                }
                else
                {
                    _components.push_back(pc);
                    lout << "ready\n";
                }
            }
        }
        else if (section == "system")
        {
            SystemApi::get()->configure(pair.second);
        }
    }
    return true;
}

bool Client::postInit()
{
    // find fs with no source content, that is the tmp fs
    Component* tmpfs = nullptr;
    Computer* pComputer = nullptr;
    for (auto* pc : components())
    {
        if (!tmpfs)
        {
            Filesystem* fs = dynamic_cast<Filesystem*>(pc);
            if (fs && fs->src() == "")
            {
                tmpfs = fs;
            }
        }
        if (!pComputer)
        {
            pComputer = dynamic_cast<Computer*>(pc);
        }
        if (pComputer && tmpfs)
        {
            pComputer->setTmpAddress(tmpfs->address());
            break;
        }
    }
    return true;
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
    lua->newlib(UnicodeApi::get());
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
    string filter = Value::check(args, 0, "string", "nil").Or("").toString();
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

    ValuePack pack = args;
    pack.erase(pack.begin(), pack.begin() + 2);
    
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

ValuePack Client::component_type(const ValuePack& args)
{
    string address = Value::check(args, 0, "string").toString();
    ValuePack result(args.state);
    Component* pc = component(address);
    if (pc)
    {
        result.push_back(pc->type());
    }
    else
    {
        result.push_back(Value::nil);
        result.push_back("no such component");
    }

    return result;
}

ValuePack Client::component_slot(const ValuePack& args)
{
    string address = Value::check(args, 0, "string").toString();
    ValuePack result(args.state);
    Component* pc = component(address);
    if (pc)
    {
        result.push_back(pc->slot());
    }
    else
    {
        result.push_back(Value::nil);
        result.push_back("no such component");
    }

    return result;
}

const string& Client::envPath() const
{
    return _env_path;
}

