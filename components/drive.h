#pragma once
#include "component.h"

#include <string>
#include <vector>

class Drive : public Component
{
public:
  Drive();
  virtual ~Drive();

  enum ConfigIndex
  {
    Tier = Component::ConfigIndex::Next,
    Label
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

  int getSectorSize();
  int getCapacity();

  int offsetToSector(int offset);
  int sectorToOffset(int sector);
  int validateSector(lua_State* lua, int sector);

  std::vector<char> read(int offset, int size);
  void write(int offset, std::vector<char> data);

private:
  std::string _hostPath; // empty when invalid
  int _tier;
  std::vector<char> _buffer;

  static bool s_registered;
};
