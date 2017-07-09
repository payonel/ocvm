#include "modem.h"
#include "drivers/modem_drv.h"
#include "model/client.h"

#include <sstream>
#include <iterator>
using std::stringstream;

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

    _modem.reset(new ModemDriver(this));
}

Modem::~Modem()
{
    _modem->stop();
}

bool Modem::onInitialize()
{
    if (!_modem->start())
    {
        return false;
    }

    _maxPacketSize = config().get(ConfigIndex::MaxPacketSize).Or(8192).toNumber();
    _maxArguments = config().get(ConfigIndex::MaxArguments).Or(8).toNumber();
    
    return true;
}

int Modem::tryPack(lua_State* lua, int offset, vector<char>* pOut) const
{
    pOut->clear();
    int last_index = lua_gettop(lua);
    int num_arguments = last_index - offset + 1;
    if (num_arguments > _maxArguments)
    {
        return luaL_error(lua, "packet has too many parts");
    }

    stringstream output(stringstream::in | stringstream::out | stringstream::binary);
    output << static_cast<int32_t>(num_arguments);

    // the accumulating packet size does not include the serialization
    // the ACTUAL packet size may be larger than what OC packs
    size_t total_packet_size = 0;
    vector<char> strValue;
    const char* pValue;
    int valueLen;

    // loop through the remaining values
    for (int index = offset; index <= last_index; ++index)
    {
        int type_id = lua_type(lua, index);
        output << type_id;
        switch (type_id)
        {
            case LUA_TSTRING:
                // size of string in bytes, +2
                pValue = lua_tostring(lua, index);
                valueLen = lua_rawlen(lua, index);
                total_packet_size += valueLen;
                total_packet_size += 2;
                output << static_cast<int32_t>(valueLen);
                std::copy(pValue, pValue + valueLen, std::ostream_iterator<char>(output));
            break;
            case LUA_TBOOLEAN:
                // size: 6
                total_packet_size += 6;
                output << lua_toboolean(lua, index);
            break;
            case LUA_TNUMBER:
                // size: 10
                total_packet_size += 10;
                output << static_cast<LUA_NUMBER>(lua_tonumber(lua, index));
            break;
            case LUA_TNIL:
                // size: 6
                total_packet_size += 6;
            break;
            default: return luaL_error(lua, "unsupported data type");
        }

        if (total_packet_size > _maxPacketSize)
        {
            std::stringstream ss;
            ss << "packet too big (max " << _maxPacketSize << ")";
            return luaL_error(lua, ss.str().c_str());
        }
    }

    auto* pbuf = output.rdbuf();
    std::size_t buffer_size = pbuf->pubseekoff(0, output.end, output.in);
    pbuf->pubseekpos(0, output.in);

    pOut->resize(buffer_size);
    pbuf->sgetn(pOut->data(), buffer_size);

    return 0;
}

int Modem::setWakeMessage(lua_State* lua)
{
    return ValuePack::ret(lua, Value::nil, "not supported");
}

int Modem::isWireless(lua_State* lua)
{
    return ValuePack::ret(lua, false);
}

int Modem::close(lua_State* lua)
{
    int port = Value::checkArg<int>(lua, 1);
    if (port < 1 || port > 0xffff)
        return luaL_error(lua, "invalid port number");
    return ValuePack::ret(lua, _modem->close(port));
}

int Modem::getWakeMessage(lua_State* lua)
{
    return ValuePack::ret(lua, Value::nil, "not supported");
}

int Modem::maxPacketSize(lua_State* lua)
{
    return ValuePack::ret(lua, _maxPacketSize);
}

int Modem::isOpen(lua_State* lua)
{
    int port = Value::checkArg<int>(lua, 1);
    if (port < 1 || port > 0xffff)
        return luaL_error(lua, "invalid port number");

    return ValuePack::ret(lua, _modem->isOpen(port));
}

int Modem::broadcast(lua_State* lua)
{
    int port = Value::checkArg<int>(lua, 1);
    if (port < 1 || port > 0xffff)
        return luaL_error(lua, "invalid port number");

    vector<char> payload;
    int ret = tryPack(lua, 2, &payload);
    if (ret)
        return ret;

    return ValuePack::ret(lua, _modem->broadcast(port, payload));
}

int Modem::send(lua_State* lua)
{
    string address = Value::checkArg<string>(lua, 1);
    int port = Value::checkArg<int>(lua, 2);
    if (port < 1 || port > 0xffff)
        return luaL_error(lua, "invalid port number");

    vector<char> payload;
    int ret = tryPack(lua, 2, &payload);
    if (ret)
        return ret;
    return ValuePack::ret(lua, _modem->send(address, port, payload));
}

int Modem::open(lua_State* lua)
{
    int port = Value::checkArg<int>(lua, 1);
    if (port < 1 || port > 0xffff)
        return luaL_error(lua, "invalid port number");
    return ValuePack::ret(lua, _modem->open(port));
}

RunState Modem::update()
{
    ModemEvent me;
    while (EventSource<ModemEvent>::pop(me))
    {
        string send_address;
        string recv_address;
        int port = 0;
        int distance = 0;
        client()->pushSignal({"modem_message", send_address, recv_address, port, distance});
    }

    return RunState::Continue;
}
