#include "drive.h"
#include "model/client.h"
#include "drivers/fs_utils.h"
#include "model/host.h"

bool Drive::s_registered = Host::registerComponentType<Drive>("drive");

Drive::Drive()
{
  add("writeByte", &Drive::writeByte, "");
  add("readByte", &Drive::readByte, "");
  add("setLabel", &Drive::setLabel, "");
  add("getSectorSize", &Drive::getSectorSize, "");
  add("getPlatterCount", &Drive::getPlatterCount, "");
  add("getLabel", &Drive::getLabel, "");
  add("writeSector", &Drive::writeSector, "");
  add("getCapacity", &Drive::getCapacity, "");
  add("readSector", &Drive::readSector, "");
}

int Drive::writeByte(lua_State* lua)
{
  return ValuePack::ret(lua);
}

int Drive::readByte(lua_State* lua)
{
  return ValuePack::ret(lua);
}

int Drive::setLabel(lua_State* lua)
{
  string new_label = Value::checkArg<string>(lua, 1);
  int stack = getLabel(lua);
  update(ConfigIndex::Label, new_label);
  return stack;
}

int Drive::getSectorSize(lua_State* lua)
{
  return ValuePack::ret(lua);
}

int Drive::getPlatterCount(lua_State* lua)
{
  return ValuePack::ret(lua);
}

int Drive::getLabel(lua_State* lua)
{
  return ValuePack::ret(lua, config().get(ConfigIndex::Label));
}

int Drive::writeSector(lua_State* lua)
{
  return ValuePack::ret(lua);
}

int Drive::getCapacity(lua_State* lua)
{
  return ValuePack::ret(lua);
}

int Drive::readSector(lua_State* lua)
{
  return ValuePack::ret(lua);
}

bool Drive::onInitialize()
{
  // the drive is always stored in the vm root dir using its component address
  _hostPath = client()->envPath() + address();
  return true;
}
