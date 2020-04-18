#pragma once

#include "component.h"
#include "io/event.h"

#include <memory>
#include <set>
#include <vector>

class ModemDriver;
using std::set;
using std::unique_ptr;
using std::vector;

class Modem : public Component, public EventSource<ModemEvent>
{
public:
  Modem();
  virtual ~Modem();

  enum ConfigIndex
  {
    SystemPort = Component::ConfigIndex::Next,
    MaxPacketSize,
    MaxArguments,
    HostAddress // defaults to 127.0.0.1
  };

  int setWakeMessage(lua_State*);
  int isWireless(lua_State*);
  int close(lua_State*);
  int getWakeMessage(lua_State*);
  int maxPacketSize(lua_State*);
  int isOpen(lua_State*);
  int broadcast(lua_State*);
  int send(lua_State*);
  int open(lua_State*);
  int setStrength(lua_State*);

protected:
  bool onInitialize() override;
  RunState update() override;
  int tryPack(lua_State* lua, const vector<char>* pAddr, int port, vector<char>* pOut) const;
  bool isApplicable(int port, vector<char>* target);

  unique_ptr<ModemDriver> _modem;
  size_t _maxPacketSize;
  int _maxArguments;

  set<int> _ports;

  static bool s_registered;
};
