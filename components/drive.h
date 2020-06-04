#pragma once
#include "component.h"

#include <string>

class Drive : public Component
{
public:
  Drive();

  enum ConfigIndex
  {
    Label = Component::ConfigIndex::Next,    
  };

  int writeByte(lua_State* lua);
  int readByte(lua_State* lua);
  int setLabel(lua_State* lua);
  int getSectorSize(lua_State* lua);
  int getPlatterCount(lua_State* lua);
  int getLabel(lua_State* lua);
  int writeSector(lua_State* lua);
  int getCapacity(lua_State* lua);
  int readSector(lua_State* lua);

protected:
  bool onInitialize() override;

private:
  std::string _hostPath; // empty when invalid
  static bool s_registered;
};
