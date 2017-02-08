#pragma once
#include "value.h"
#include "luaproxy.h"

#include <string>
#include <vector>

class Host;
class LuaEnv;
class Component;
class Config;

class Client : public LuaProxy
{
public:
    Client(Host*);
    ~Client();
    bool load(LuaEnv*);
    void close();

    // global api that is actually computer specific
    // invoke by address
    ValuePack component_invoke(const ValuePack& args);
    ValuePack component_list(const ValuePack& args);
    ValuePack component_methods(const ValuePack& args);
protected:
    bool loadLuaComponentApi(LuaEnv*);
private:
    std::vector<Component*> _components;
    Config* _config;
    Host* _host;
};
