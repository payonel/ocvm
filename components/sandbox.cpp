#include "sandbox.h"
#include "computer.h"
#include "drivers/fs_utils.h"
#include "model/client.h"
#include "model/host.h"
#include "model/log.h"

bool Sandbox::s_registered = Host::registerComponentType<Sandbox>("sandbox");

Sandbox::Sandbox()
{
  add("add_component", &Sandbox::add_component);
  add("remove_component", &Sandbox::remove_component);
  add("log", &Sandbox::log);
  add("state_name", &Sandbox::state_name);
}

int Sandbox::log(lua_State* lua)
{
  return client()->computer()->print(lua);
}

int Sandbox::state_name(lua_State* lua)
{
  string stateName = fs_utils::filename(client()->envPath());
  return ValuePack::ret(lua, stateName);
}

int Sandbox::add_component(lua_State* lua)
{
  Value component_config(lua, 1);
  if (component_config.len() == 0 || component_config.get(1).type() != "string")
  {
    return luaL_error(lua, "missing component type name");
  }

  if (!client()->add_component(component_config))
  {
    string err = "failed to add component: " + component_config.serialize();
    return luaL_error(lua, err.c_str());
  }

  return ValuePack::ret(lua, component_config.get(2).toString());
}

int Sandbox::remove_component(lua_State* lua)
{
  string address = Value::checkArg<string>(lua, 1);
  if (!client()->remove_component(address))
  {
    string err = "Failed to remove component: " + address;
    return luaL_error(lua, err.c_str());
  }

  return ValuePack::ret(lua, true);
}

bool Sandbox::onInitialize()
{
  return true;
}
