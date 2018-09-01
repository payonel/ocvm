#include "internet_drv.h"

#include <algorithm>
#include <string.h>

#include "model/log.h"

class TcpObject : public InternetConnection
{
public:
    TcpObject(const string& addr, int port);
    int write(lua_State* lua);
protected:
    Connection* connection() const override;
private:
    unique_ptr<Connection> _connection;
};

InternetConnection::InternetConnection() :
    _needs_connection(false),
    _needs_data(false)
{
    add("finishConnect", &InternetConnection::finishConnect);
    add("read", &InternetConnection::read);
    add("close", &InternetConnection::close);
}

void InternetConnection::setOnClose(InternetConnectionEventSet::OnClosedCallback cb)
{
    handler.onClosed = cb;
}

void InternetConnection::dispose()
{
    _close();
    if (handler.onClosed)
        handler.onClosed(this);
}

int InternetConnection::finishConnect(lua_State* lua)
{
    _needs_connection = connection()->state() < ConnectionState::Ready;
    return ValuePack::ret(lua, !_needs_connection);
}

int InternetConnection::close(lua_State* lua)
{
    _close();
    return 0;
}

void InternetConnection::_close()
{
    connection()->close();
}

bool InternetConnection::update()
{
    bool updated = false;
    if (_needs_connection)
    {
        if (connection()->state() >= ConnectionState::Ready)
        {
            _needs_connection = false;
            updated = true;
        }
    }
    if (_needs_data)
    {
        if (connection()->state() == ConnectionState::Ready)
        {
            auto avail = connection()->bytes_available();
            if (connection()->preload(avail + 1))
            {
                _needs_data = false;
                updated = true;
            }
        }
    }

    return updated;
}

int InternetConnection::read(lua_State* lua)
{
    _needs_data = true;
    if (!connection()->can_read())
        return ValuePack::ret(lua, Value::nil, "not connected");

    LUA_NUMBER default_n = INT_MAX;
    ssize_t n = static_cast<ssize_t>(Value::checkArg<LUA_NUMBER>(lua, 1, &default_n));

    connection()->preload(n);
    n = std::min(n, connection()->bytes_available());
    vector<char> buffer;
    connection()->back_insert(&buffer, 0, n);
    connection()->move(buffer.size());

    if (buffer.size() == 0 && connection()->state() == ConnectionState::Finished)
    {
        connection()->close();
        return ValuePack::ret(lua, Value::nil);
    }

    return ValuePack::ret(lua, buffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

TcpObject::TcpObject(const string& addr, int port)
{
    add("write", &TcpObject::write);
    this->name("TcpObject");

    _connection.reset(new Connection(addr, port));
}

int TcpObject::write(lua_State* lua)
{
    if (!_connection->can_write())
        return ValuePack::ret(lua, Value::nil, "not connected");

    vector<char> buffer = Value::checkArg<vector<char>>(lua, 1);
    _connection->write(buffer);
    return ValuePack::ret(lua, buffer.size());
}

Connection* TcpObject::connection() const
{
    return _connection.get();
}

/////////////////////////////////////////////////////////////////////////////////////////////////

HttpAddress::HttpAddress(string url) :
    raw(url),
    valid(false)
{
    size_t protocol_end = url.find("://");
    if (protocol_end == string::npos)
        return;

    string protocol = url.substr(0, protocol_end);
    if (protocol == "http")
    {
        this->https = false;
        this->port = 80;
    }
    else if (protocol == "https")
    {
        this->https = true;
        this->port = 443;
    }
    else
        return;

    url = url.substr(protocol_end + 3); // remove protocol and ://

    size_t params_index = url.find("?");
    if (params_index != string::npos)
    {
        this->params = url.substr(params_index + 1);
        url = url.substr(0, params_index);
    }

    this->hostname = url.substr(0, url.find("/"));

    if (this->hostname.empty())
        return;

    if (url.size() > this->hostname.size())
        this->path = url.substr(this->hostname.size() + 1);

    // remove port from hostname if it exists
    size_t port_index = this->hostname.find(":");
    if (port_index != string::npos)
    {
        string port_text = this->hostname.substr(port_index + 1);
        this->hostname = this->hostname.substr(0, port_index);
        stringstream ss;
        ss << port_text;
        ss >> this->port;
        if (!ss || !ss.eof())
        {
            // invalid port number
            return;
        }
    }

    this->valid = true;
}

InternetConnection::HttpGenRegistry& InternetConnection::http_gen()
{
    static HttpGenRegistry s_http = nullptr;
    return s_http;
}

InternetConnection* InternetConnection::openTcp(UserDataAllocator allocator, const InternetConnectionEventSet& eventSet, const TcpConstructionParameters& args)
{
    UserData* pAlloc = allocator(sizeof(TcpObject));
    auto pConn = new(pAlloc) TcpObject(args.addr, args.port);
    pConn->setOnClose(eventSet.onClosed);
    return pConn;
}

InternetConnection* InternetConnection::openHttp(UserDataAllocator allocator, const InternetConnectionEventSet& eventSet, const HttpConstructionParameters& args)
{
    if (InternetConnection::http_gen() == nullptr)
        return nullptr;
    InternetConnection* pConn = InternetConnection::http_gen()(allocator, args);
    pConn->setOnClose(eventSet.onClosed);
    return pConn;
}
