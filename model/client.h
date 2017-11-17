#pragma once
#include "value.h"
#include "luaproxy.h"
#include "model/log.h"

#include <string>
#include <vector>

class Host;
class Config;
class Component;
class Computer;
class SandboxMethods;
enum class RunState;

class Client : public LuaProxy
{
public:
    Client(Host*, const string& env_path);
    ~Client();
    bool load();
    void close();
    vector<Component*> components(string filter = "", bool exact = false) const;
    Component* component(const string& address) const;
    const string& envPath() const;
    Host* host() const;
    void computer(Computer*);
    Computer* computer() const;
    void pushSignal(const ValuePack& pack);
    RunState run();
    bool add_component(Value& component_config);
    bool remove_component(const string& address);

    void append_crash(const string& report);
    Logger& lout();

    // global api that is actually computer specific
    // invoke by address
    int component_invoke(lua_State* lua);
    int component_list(lua_State* lua);
    int component_methods(lua_State* lua);
    int component_type(lua_State* lua);
    int component_slot(lua_State* lua);
    int component_doc(lua_State* lua);
protected:
    bool createComponents();
    bool postInit();
    bool loadLuaComponentApi();
private:
    vector<Component*> _components;
    Computer* _computer;
    Config* _config;
    string _env_path;
    Host* _host;
    SandboxMethods* _globals;
    string _crash;
    Logger _lout;
};
