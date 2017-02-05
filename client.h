#pragma once
#include "frame.h"

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
private:
    std::vector<Component*> _components;
    Config* _config;
    Host* _host;
};
