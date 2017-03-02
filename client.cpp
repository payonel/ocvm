#include "client.h"
#include "host.h"
#include "components/component.h"
#include "components/computer.h"

#include "config.h"
#include "log.h"
#include "utils.h"
#include <lua.hpp>

#include "apis/os.h"
#include "apis/global_methods.h"
#include "apis/system.h"
#include "apis/unicode.h"
#include "apis/sandbox_methods.h"

#include <string>
#include <functional>

using std::make_tuple;

Client::Client(Host* host, const string& env_path) : 
    LuaProxy("component"),
    _computer(nullptr),
    _config(nullptr),
    _env_path(env_path),
    _host(host),
    _globals(nullptr)
{
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

Host* Client::host() const
{
    return _host;
}

bool Client::load()
{
    if (_config || _globals)
    {
        lout << "Client is either already loaded or did not close properly";
        return false;
    }

    _config = new Config();
    _globals = new SandboxMethods(this);

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
    lout << "components post initialized\n";

    if (!loadLuaComponentApi())
    {
        lout << "failed to load lua component api\n";
        return false;
    }

    return _computer->load(host()->machinePath());
}

bool Client::createComponents()
{
    // load components from config
    for (const auto& section : _config->keys())
    {
        auto& section_data = _config->get(section);
        if (section == "components")
        {
            int count = section_data.len();
            for (int index = 1; index <= count; index++)
            {
                if (!section_data.contains(index))
                    continue;
                Value& component_config = section_data.get(index);
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
            SystemApi::configure(section_data);
        }
        else if (section == "unicode")
        {
            UnicodeApi::configure(section_data);
        }
    }
    return true;
}

bool Client::postInit()
{
    // find fs with no source content, that is the tmp fs
    for (auto* pc : components())
    {
        if (!pc->postInit())
        {
            lout << pc->type() << "[" << pc->address() << "] failed to postInit\n";
            return false;
        }
        // machine.lua handles component_added for us
        // _computer->pushSignal(ValuePack({"component_added", pc->address(), pc->type()}));
    }

    return true;
}

bool Client::loadLuaComponentApi()
{
    // computer required
    if (!_computer)
    {
        lout << "emulation requires exactly one computer component\n";
        return false;
    }

    _computer->newlib(this);
    _computer->newlib(_globals);
    _computer->newlib(OSApi::get());
    _computer->newlib(GlobalMethods::get());
    _computer->newlib(SystemApi::get());
    _computer->newlib(UnicodeApi::get());
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
        
    delete _globals;
    _globals = nullptr;

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

int Client::component_list(lua_State* lua)
{
    string filter = Value::check(lua, 1, "string", "nil").Or("").toString();
    bool exact = Value::check(lua, 2, "boolean", "nil").toBool();

    Value result = Value::table();
    for (auto* pc : components(filter, exact))
    {
        result.set(pc->address(), pc->type());
    }

    return ValuePack::ret(lua, result);
}

int Client::component_invoke(lua_State* lua)
{
    // for logging, this is called via LuaProxy because all method calls are dispatched there first
    // LuaProxy::invoke has already logged much about this call, but is waiting to log the result
    // but, logging from here on out will look like a return value, so we add some indentation here
    // lout << "-> ";
    string address = Value::check(lua, 1, "string").toString();
    lua_remove(lua, 1);
    string methodName = Value::check(lua, 1, "string").toString();
    lua_remove(lua, 1);
    
    Component* pc = component(address);
    if (!pc)
        return ValuePack::ret(lua, Value::nil, "no such component " + address);

    int stacked = pc->invoke(methodName, lua);
    lua_pushboolean(lua, true);
    lua_insert(lua, 1);
    return stacked + 1;
}

int Client::component_methods(lua_State* lua)
{
    string address = Value::check(lua, 1, "string").toString();

    Component* pc = component(address);
    if (!pc)
        return ValuePack::ret(lua, Value::nil, "no such component");

    Value mpack = Value::table();
    Value info = Value::table();
    info.set("direct", true);
    for (const auto& luaMethod : pc->methods())
    {
        mpack.set(std::get<0>(luaMethod), info);
    }
    return ValuePack::ret(lua, mpack);
}

int Client::component_type(lua_State* lua)
{
    string address = Value::check(lua, 1, "string").toString();
    Component* pc = component(address);
    if (!pc)
        return ValuePack::ret(lua, Value::nil, "no such component");

    return ValuePack::ret(lua, pc->type());
}

int Client::component_slot(lua_State* lua)
{
    string address = Value::check(lua, 1, "string").toString();
    Component* pc = component(address);
    if (!pc)
        return ValuePack::ret(lua, Value::nil, "no such component");

    return ValuePack::ret(lua, pc->slot());
}

const string& Client::envPath() const
{
    return _env_path;
}

void Client::computer(Computer* c)
{
    _computer = c;
}

Computer* Client::computer() const
{
    return _computer;
}

RunState Client::run()
{
    for (auto& pc : _components)
    {
        auto state = pc->update();
        if (state != RunState::Continue)
        {
            return state;
        }
    }

    return RunState::Continue;
}

void Client::pushSignal(const ValuePack& pack)
{
    _computer->pushSignal(pack);
}
