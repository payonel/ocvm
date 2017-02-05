#include "client.h"
#include "host.h"
#include "components/component.h"
#include "config.h"
#include "log.h"

#include <string>

using std::endl;
using std::string;

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

    for (auto pc : _components)
    {
        if (pc->type() == "gpu")
        {
            pc->invoke("setResolution", 50, 16);
        }
    }

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
