#include "drive.h"
#include "model/client.h"
#include "drivers/fs_utils.h"
#include "model/host.h"
#include "model/log.h"
using Logging::lout;

#include <fstream>
#include <string.h>

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

Drive::~Drive()
{
  std::ofstream writer(_hostPath);
  writer.write(_buffer.data(), _buffer.size());
  writer.close();
}

int Drive::writeByte(lua_State* lua)
{
  int offset = Value::checkArg<int>(lua, 1);
  int byte = Value::checkArg<int>(lua, 2);
  int sector = offsetToSector(offset);
  validateSector(lua, sector);
  std::vector<char> data;
  data.push_back(byte);
  write(offset, data);
  return ValuePack::ret(lua);
}

int Drive::readByte(lua_State* lua)
{
  int offset = Value::checkArg<int>(lua, 1);
  int sector = offsetToSector(offset);
  validateSector(lua, sector);

  auto data = read(offset, 1);
  assert(data.size() == 1);
  return ValuePack::ret(lua, static_cast<int>(data.at(0)));
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
  return ValuePack::ret(lua, getSectorSize());
}

int Drive::getPlatterCount(lua_State* lua)
{
  int count;
  switch (_tier)
  {
    case 1: count = 2; break;
    case 2: count = 4; break;
    case 3: count = 6; break;
    default: count = 0;
  }

  return ValuePack::ret(lua, count);
}

int Drive::getLabel(lua_State* lua)
{
  return ValuePack::ret(lua, config().get(ConfigIndex::Label));
}

int Drive::writeSector(lua_State* lua)
{
  int sector = Value::checkArg<int>(lua, 1);
  std::vector<char> data = Value::checkArg<std::vector<char>>(lua, 2);
  validateSector(lua, sector);
  data.resize(getSectorSize());
  int offset = sectorToOffset(sector);
  write(offset, data);
  return ValuePack::ret(lua);
}

int Drive::getCapacity(lua_State* lua)
{
  return ValuePack::ret(lua, getCapacity());
}

int Drive::readSector(lua_State* lua)
{
  int sector = Value::checkArg<int>(lua, 1) - 1;
  validateSector(lua, sector);
  int offset = sectorToOffset(sector);
  std::vector<char> data = read(offset, getSectorSize());
  assert(data.size() == static_cast<size_t>(getSectorSize()));
  return ValuePack::ret(lua, data);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

bool Drive::onInitialize()
{
  // the drive is always stored in the vm root dir using its component address
  _hostPath = client()->envPath() + "/" + address();
  Value tier = config().get(ConfigIndex::Tier).Or(3);
  if (tier.type() != "number")
  {
    return false;
  }
  _tier = tier.toNumber();

  if (fs_utils::exists(_hostPath))
  {
    fs_utils::read(_hostPath, _buffer);
    _buffer.resize(getCapacity());
  }
  else
  {
    _buffer.resize(getCapacity());
    std::ofstream file(_hostPath, std::ios_base::app | std::ios_base::out);
    if (!file)
    {
      lout << "Failed to initialize drive[" << address() << "] file" << std::endl;
      return false;
    }
    file.close();
  }

  return true;
}

int Drive::getSectorSize()
{
  return 512;
}

int Drive::getCapacity()
{
  int kbs;
  switch (_tier)
  {
    case 1: kbs = 1024; break;
    case 2: kbs = 2048; break;
    case 3: kbs = 4096; break;
    default: kbs = 0;
  }
  return kbs * 1024;
}

int Drive::offsetToSector(int offset)
{
  return offset / getSectorSize();
}

int Drive::sectorToOffset(int sector)
{
  return sector * getSectorSize();
}

int Drive::validateSector(lua_State* lua, int sector)
{
  int offset = sectorToOffset(sector);
  if (sector < 0 || offset >= getCapacity())
  {
    return luaL_error(lua, "invalid offset, not in a usable sector");
  }

  return 0;
}

std::vector<char> Drive::read(int offset, int size)
{
  assert(offset >= 0);
  assert(size >= 0);
  if (size == 0) return {};

  if (static_cast<size_t>(offset) >= _buffer.size())
  {
    return {};
  }

  size_t expectedAvail = static_cast<size_t>(offset + size);
  if (expectedAvail > _buffer.size())
  {
    size = _buffer.size() - offset;
  }

  std::vector<char> buffer;
  buffer.resize(size);
  ::memcpy(buffer.data(), _buffer.data() + offset, buffer.size());
  return buffer;
}

void Drive::write(int offset, std::vector<char> data)
{
  assert(offset >= 0);
  size_t uoffset = static_cast<size_t>(offset);
  if (uoffset >= _buffer.size())
  {
    lout << "bad drive write beyond buffer" << std::endl;
    return;
  }

  size_t limit = _buffer.size() - uoffset;
  size_t write_size = std::min(limit, data.size());

  ::memcpy(_buffer.data() + uoffset, data.data(), write_size);
}
