#pragma once
#include "component.h"

class Client;

class Sandbox : public Component
{
public:
    Sandbox();

    int add_component(lua_State* lua);
    int remove_component(lua_State* lua);
    int log(lua_State* lua);
  int state_name(lua_State* lua);
protected:
    bool onInitialize() override;

private:
    static bool s_registered;
};
