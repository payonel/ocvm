#include "internet.h"
#include "model/client.h"
#include "apis/system.h"
#include "model/host.h"

#include "drivers/internet_drv.h"

#include <sstream>
using std::stringstream;

bool Internet::s_registered = Host::registerComponentType<Internet>("internet");

Internet::Internet()
{
    add("isTcpEnabled", &Internet::isTcpEnabled);
    add("isHttpEnabled", &Internet::isHttpEnabled);
    add("connect", &Internet::connect);
    add("request", &Internet::request);
}

Internet::~Internet()
{
    while (!_connections.empty())
        release(*_connections.begin());
}

bool Internet::onInitialize()
{
    _tcp = config().get(ConfigIndex::TcpEnabled).Or(true).toBool();
    _http = config().get(ConfigIndex::HttpEnabled).Or(true).toBool();
    return true;
}

int Internet::isTcpEnabled(lua_State* lua)
{
    return ValuePack::ret(lua, _tcp);
}

int Internet::isHttpEnabled(lua_State* lua)
{
    return ValuePack::ret(lua, _http);
}

int Internet::connect(lua_State* lua)
{
    string addr = Value::checkArg<string>(lua, 1);
    int default_port = -1;
    int port = Value::checkArg<int>(lua, 2, &default_port);
    if (!_tcp)
    {
        return ValuePack::ret(lua, Value::nil, "tcp connections are unavailable");
    }
    if (static_cast<int>(_connections.size()) >= SystemApi::max_connections())
    {
        return luaL_error(lua, "too many open connections");
    }

    int parsed_port;
    if (parsePort(&addr, &parsed_port))
    {
        port = parsed_port;
    }

    if (port < 0 || port >= 0xffff)
    {
        return luaL_error(lua, "address could not be parsed or no valid port given");
    }

    lua_settop(lua, 0);
    void* pAlloc = lua_newuserdata(lua, sizeof(TcpObject));
    TcpObject* pConn = new(pAlloc) TcpObject(addr, port);
    pConn->setOnClose([this](InternetConnection* _pc){
        this->release(_pc);
    });
    _connections.insert(pConn);

    return 1;
}

int Internet::request(lua_State* lua)
{
    string addr = Value::checkArg<string>(lua, 1);
    static const string default_post = "";
    string post = Value::checkArg<string>(lua, 2, &default_post);
    map<string, string> header;
    header = Value::checkArg<decltype(header)>(lua, 3, &header);

    if (!_http)
    {
        return ValuePack::ret(lua, Value::nil, "http connections are unavailable");
    }
    HttpAddress httpAddr(addr);
    if (!httpAddr.valid)
    {
        return ValuePack::ret(lua, Value::nil, "invalid address");
    }
    if (static_cast<int>(_connections.size()) >= SystemApi::max_connections())
    {
        return luaL_error(lua, "too many open connections");
    }

    lua_settop(lua, 0);
    void* pAlloc = lua_newuserdata(lua, sizeof(HttpObject));
    HttpObject* pConn = new(pAlloc) HttpObject(httpAddr, post, header);
    pConn->setOnClose([this](InternetConnection* _pc){
        this->release(_pc);
    });
    _connections.insert(pConn);

    return 1;
}

bool Internet::parsePort(string* pAddr, int* pPort) const
{
    size_t c_index = pAddr->find_last_of(":");
    if (c_index == string::npos)
        return false;

    stringstream ss;
    ss << pAddr->substr(c_index + 1);
    int port;
    ss >> port;

    if (!ss || !ss.eof() || port < 0 || port >= 0xffff)
        return false;

    *pAddr = pAddr->substr(0, c_index);
    *pPort = port;
    return true;
}

bool Internet::release(InternetConnection* pConn)
{
    return _connections.erase(pConn) > 0;
}

RunState Internet::update()
{
    for (InternetConnection* inc : _connections)
    {
        if (inc->update())
        {
            ValuePack pack {"internet_ready", address()};
            client()->pushSignal(pack);
        }
    }

    return RunState::Continue;
}
