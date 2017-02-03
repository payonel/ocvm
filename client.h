#pragma once
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
    bool load(LuaEnv*);
    void close();
private:
    std::vector<Component*> _components;
    Config* _config;
    Host* _host;
};
