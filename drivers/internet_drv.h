#pragma once

#include "apis/userdata.h"
#include "connection.h"
#include "io/event.h"
#include <memory>
#include <functional>
using std::unique_ptr;

struct HttpAddress
{
public:
    explicit HttpAddress(string url);

    string raw;
    bool valid;
    bool https;
    string hostname;
    string path;
    string params;
    int port;
};

class InternetConnection;
struct InternetConnectionEventSet
{
    using OnClosedCallback = std::function<void(InternetConnection*)>;
    OnClosedCallback onClosed = nullptr;
};

struct TcpConstructionParameters
{
    string addr;
    int port;
};

struct HttpConstructionParameters
{
    HttpAddress addr;
    string post;
    map<string, string> header;
};

class InternetConnection : public UserData
{
public:
    InternetConnection();

    void dispose() override;

    int finishConnect(lua_State* lua);
    int read(lua_State* lua);
    int close(lua_State* lua);

    virtual bool update();

    void setOnClose(InternetConnectionEventSet::OnClosedCallback cb);

    using HttpGenRegistry = std::function<InternetConnection*(UserDataAllocator allocator, const HttpConstructionParameters&)>;

    static HttpGenRegistry& http_gen();

    static InternetConnection* openTcp(UserDataAllocator allocator, const InternetConnectionEventSet& eventSet, const TcpConstructionParameters& args);
    static InternetConnection* openHttp(UserDataAllocator allocator, const InternetConnectionEventSet& eventSet, const HttpConstructionParameters& args);
protected:
    void open(const string& addr, int port);
    virtual Connection* connection() const = 0;

    InternetConnectionEventSet handler;
    bool _needs_connection;
    bool _needs_data;
    
    virtual void _close();
};


