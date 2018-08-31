#include "model/host.h"
#include "modem.h"
#include "drivers/modem_drv.h"
#include "model/client.h"
#include "model/log.h"

#include <sstream>
#include <iterator>
using std::stringstream;

bool Modem::s_registered = Host::registerComponentType<Modem>("modem");

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
    add("setStrength", &Modem::setStrength);
}

Modem::~Modem()
{
    _modem->stop();
}

bool Modem::onInitialize()
{
    int system_port = config().get(ConfigIndex::SystemPort).Or(56000).toNumber();
    string hostAddress = config().get(ConfigIndex::HostAddress).Or("127.0.0.1").toString();
    _modem.reset(new ModemDriver(this, system_port, hostAddress));
    if (!_modem->start())
    {
        lout() << "modem driver failed to start\n";
        return false;
    }

    _maxPacketSize = config().get(ConfigIndex::MaxPacketSize).Or(8192).toNumber();
    _maxArguments = config().get(ConfigIndex::MaxArguments).Or(8).toNumber();
    
    return true;
}

template<typename T>
void write(const T& num, vector<char>* pOut)
{
    const char* p = reinterpret_cast<const char*>(&num);
    for (size_t i = 0; i < sizeof(T); i++)
    {
        pOut->push_back(*p++);
    }
}

void write(const string& text, vector<char>* pOut)
{
    write<int32_t>(text.size(), pOut);
    std::copy(text.begin(), text.end(), std::back_inserter(*pOut));
}

void write(const char* data, int len, vector<char>* pOut)
{
    write<int32_t>(len, pOut);
    std::copy(data, data + len, std::back_inserter(*pOut));
}

int Modem::tryPack(lua_State* lua, const vector<char>* pAddr, int port, vector<char>* pOut) const
{
    if (port < 1 || port > 0xffff)
        return luaL_error(lua, "invalid port number");

    pOut->clear();

    int offset = 2; // first is at least the port
    int last_index = lua_gettop(lua);

    // {sender, has_target(1 or 0), target, port, num_args, arg_type_id_1, arg_value_1, ..., arg_type_id_n, arg_value_n)
    string addr = address();
    write(addr, pOut);
    
    if (pAddr)
    {
        offset++;
        write<bool>(true, pOut);
        write(pAddr->data(), pAddr->size(), pOut);
    }
    else
    {
        write<bool>(false, pOut);
    }

    write<int32_t>(port, pOut);

    int num_arguments = last_index - offset + 1;
    if (num_arguments > _maxArguments)
    {
        return luaL_error(lua, "packet has too many parts");
    }

    write<int32_t>(num_arguments, pOut);

    // the accumulating packet size does not include the serialization
    // the ACTUAL packet size may be larger than what OC packs
    size_t total_packet_size = 0;
    const char* pValue;
    int valueLen;

    // loop through the remaining values
    for (int index = offset; index <= last_index; ++index)
    {
        int type_id = lua_type(lua, index);
        write<int32_t>(type_id, pOut);
        switch (type_id)
        {
            case LUA_TNIL:
                // size: 6
                total_packet_size += 6;
            break;
            case LUA_TBOOLEAN:
                // size: 6
                total_packet_size += 6;
                write<bool>(lua_toboolean(lua, index), pOut);
            break;
            case LUA_TNUMBER:
                // size: 10
                total_packet_size += 10;
                write<LUA_NUMBER>(lua_tonumber(lua, index), pOut);
            break;
            case LUA_TSTRING:
                // size of string in bytes, +2
                pValue = lua_tostring(lua, index);
                valueLen = lua_rawlen(lua, index);
                total_packet_size += valueLen;
                total_packet_size += 2;
                write(pValue, valueLen, pOut);
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
    bool changed = false;
    if (_ports.find(port) != _ports.end())
    {
        changed = true;
        _ports.erase(port);
    }
    return ValuePack::ret(lua, changed);
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
    return ValuePack::ret(lua, _ports.find(port) != _ports.end());
}

int Modem::broadcast(lua_State* lua)
{
    int port = Value::checkArg<int>(lua, 1);
    vector<char> payload;
    int ret = tryPack(lua, nullptr, port, &payload);
    if (ret)
        return ret;
    return ValuePack::ret(lua, _modem->send(payload));
}

int Modem::send(lua_State* lua)
{
    vector<char> address = Value::checkArg<vector<char>>(lua, 1);
    int port = Value::checkArg<int>(lua, 2);
    vector<char> payload;
    int ret = tryPack(lua, &address, port, &payload);
    if (ret)
        return ret;
    return ValuePack::ret(lua, _modem->send(payload));
}

int Modem::open(lua_State* lua)
{
    int port = Value::checkArg<int>(lua, 1);
    if (port < 1 || port > 0xffff)
        return luaL_error(lua, "invalid port number");
    bool changed = false;
    if (_ports.find(port) == _ports.end())
    {
        changed = true;
        _ports.insert(port);
    }
    return ValuePack::ret(lua, changed);
}

template<typename T>
bool read_next(const char** pInput, const char* const end, T* pOut)
{
    if (pOut && *pInput + sizeof(T) <= end)
    {
        const char*& input = *pInput;
        char* p = reinterpret_cast<char*>(pOut);
        for (size_t i = 0; i < sizeof(T); i++)
        {
            *p++ = *input++;
        }
        return true;
    }
    return false;
}

bool read_vector(const char** pInput, const char* const end, vector<char>* pOut)
{
    if (pOut == nullptr)
        return false;
    pOut->clear();

    int size;
    if (!read_next<int32_t>(pInput, end, &size))
        return false;

    if (*pInput + size > end)
        return false;

    std::copy(*pInput, *pInput + size, std::back_inserter(*pOut));
    *pInput += size;

    return true;
}

RunState Modem::update()
{
    ModemEvent me;
    while (EventSource<ModemEvent>::pop(me))
    {
        // broadcast packets have no target
        // {sender, has_target(1 or 0)[, target], port, num_args, arg_type_id_1, arg_value_1, ..., arg_type_id_n, arg_value_n)
        const char* input = me.payload.data();
        const char* const end = input + me.payload.size();
        vector<char> send_address;
        if (!read_vector(&input, end, &send_address))
        {
            lout() << "Malformed modem packet. Could not read send_address\n";
            continue;
        }
        bool has_target;
        if (!read_next<bool>(&input, end, &has_target))
        {
            lout() << "Malformed modem packet. Could not read has_target\n";
            continue;
        }
        vector<char> recv_address;
        if (has_target)
        {
            if (!read_vector(&input, end, &recv_address))
            {
                lout() << "Malformed modem packet. Could not read recv_address\n";
                continue;
            }
        }
        int port;
        if (!read_next<int32_t>(&input, end, &port))
        {
            lout() << "Malformed modem packet. Could not read port\n";
            continue;
        }

        if (!isApplicable(port, has_target ? &recv_address : nullptr))
        {
            continue;
        }

        int distance = 0; // always zero in simulation
        ValuePack pack {"modem_message", address(), send_address, port, distance};

        int num_args;
        if (!read_next<int32_t>(&input, end, &num_args))
        {
            lout() << "Malformed modem packet. Could not read num_args\n";
            continue;
        }
        for (int n = 0; n < num_args; n++)
        {
            int type_id;
            if (!read_next<int32_t>(&input, end, &type_id))
            {
                lout() << "Malformed modem packet. Could not read type_id\n";
                continue;
            }
            // switch variables
            vector<char> string_arg;
            bool bool_arg;
            LUA_NUMBER number_arg;
            Value v;

            switch (type_id)
            {
                case LUA_TSTRING:
                    if (!read_vector(&input, end, &string_arg))
                    {
                        lout() << "Malformed modem packet. Could not read argument [" << pack.size() << "]\n";
                        continue;
                    }
                    v = string_arg;
                break;
                case LUA_TBOOLEAN:
                    if (!read_next<bool>(&input, end, &bool_arg))
                    {
                        lout() << "Malformed modem packet. Could not read argument [" << pack.size() << "]\n";
                        continue;
                    }
                    v = bool_arg;
                break;
                case LUA_TNUMBER:
                    if (!read_next<LUA_NUMBER>(&input, end, &number_arg))
                    {
                        lout() << "Malformed modem packet. Could not read argument [" << pack.size() << "]\n";
                        continue;
                    }
                    v = number_arg;
                break;
                case LUA_TNIL:
                    v = Value::nil;
                break;
            }
            pack.emplace_back(v);
        }

        client()->pushSignal(pack);
    }

    return RunState::Continue;
}

bool Modem::isApplicable(int port, vector<char>* target)
{
    if (_ports.find(port) == _ports.end())
    {
        return false;
    }

    if (target)
    {
        string addr = address();
        if (addr.size() != target->size())
            return false;

        for (size_t i = 0; i < addr.size(); i++)
        {
            if (addr.at(i) != target->at(i))
                return false;
        }
    }

    return true;
}

int Modem::setStrength(lua_State* lua)
{
    return ValuePack::ret(lua, Value::nil, "not implemented");
}

