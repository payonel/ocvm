#pragma once

#include "component.h"
#include "io/event.h"

#include <memory>
#include <vector>

class ModemDriver;
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
        MaxArguments
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
protected:
    bool onInitialize() override;
    RunState update() override;
    int tryPack(lua_State* lua, const vector<char>* pAddr, int port, vector<char>* pOut) const;

    unique_ptr<ModemDriver> _modem;
    size_t _maxPacketSize;
    int _maxArguments;
};
