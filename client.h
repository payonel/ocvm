#pragma once
#include "value.h"

#include <string>
#include <vector>

class Host;
class LuaEnv;
class Component;
class Config;

class Client
{
public:
    Client(Host*);
    ~Client();
    bool load(LuaEnv*);
    void close();

    // global api that is actually computer specific
    // invoke by address
    std::vector<Component*> component_list(const std::string& filter = "", bool exact = false);
    ValuePack component_invoke(const std::string& address, const std::string& methodName, const ValuePack& args);
private:
    std::vector<Component*> _components;
    Config* _config;
    Host* _host;
};
