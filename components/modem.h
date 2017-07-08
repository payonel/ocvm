#pragma once

#include "component.h"

class Modem : public Component
{
public:
    Modem();

    enum ConfigIndex
    {
        SystemPort = Component::ConfigIndex::Next,
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
};
