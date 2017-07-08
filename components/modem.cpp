#include "modem.h"

Modem::Modem()
{
    add("setWakeMessage", &Modem::setWakeMessage);
    add("isWireless", &Modem::isWireless);
    add("close", &Modem::close);
    add("getWakeMessage", &Modem::getWakeMessage);
    add("maxPacketSize", &Modem::maxPacketSize);
    add("isOpen", &Modem::isOpen);
    add("broadcast", &Modem::broadcast);
    add("send", &Modem::send);
    add("open", &Modem::open);
}

bool Modem::onInitialize()
{
    return true;
}

int Modem::setWakeMessage(lua_State* lua)
{
    return 0;
}

int Modem::isWireless(lua_State* lua)
{
    return 0;
}

int Modem::close(lua_State* lua)
{
    return 0;
}

int Modem::getWakeMessage(lua_State* lua)
{
    return 0;
}

int Modem::maxPacketSize(lua_State* lua)
{
    return 0;
}

int Modem::isOpen(lua_State* lua)
{
    return 0;
}

int Modem::broadcast(lua_State* lua)
{
    return 0;
}

int Modem::send(lua_State* lua)
{
    return 0;
}

int Modem::open(lua_State* lua)
{
    return 0;
}
