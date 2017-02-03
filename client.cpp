#include "client.h"
#include "host.h"
#include "component.h"
#include "config.h"
#include "log.h"

Client::Client(Host* host) : _host(host)
{
    _config = new Config(host->envPath(), "client");
}

bool Client::load(LuaEnv* lua)
{
    // load components from config
    for (auto key : _config->keys())
    {
        log << key << ": ";
        Component* pc = _host->create(key);
        if (pc)
        {
            log << "created\n";
            _components.push_back(pc);
        }
        else
        {
            log << "failed\n";
        }
    }

    if (!_components.empty())
    {
        _components.at(0)->invoke("test");
    }

    return true;
}

void Client::close()
{
    _config->save();
    delete _config;

    for (auto pc : _components)
        delete pc;

    _components.clear();
}
