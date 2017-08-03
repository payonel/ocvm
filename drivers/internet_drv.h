#pragma once

#include "apis/userdata.h"
#include "connection.h"
#include "io/event.h"
#include <memory>
using std::unique_ptr;

struct HttpAddress
{
public:
    HttpAddress(string url);

    string raw;
    bool valid;
    bool https;
    string hostname;
    string path;
    string params;
    int port;
};

class Internet;
class InternetConnection : public UserData
{
public:
    InternetConnection(Internet* inet);

    void dispose() override;

    int finishConnect(lua_State* lua);
    int read(lua_State* lua);
    int close(lua_State* lua);

    virtual bool update();

protected:
    void open(const string& addr, int port);

    Internet* _inet;
    unique_ptr<Connection> _connection;
    bool _needs_connection;
    bool _needs_data;

    virtual void _close();
};

class TcpObject : public InternetConnection
{
public:
    TcpObject(Internet* inet, const string& addr, int port);
    int write(lua_State* lua);
};

class PipedCommand
{
public:
    PipedCommand();
    virtual ~PipedCommand();
    bool open(const string& command, const vector<string>& args);
    int stdin() const;
    int stdout() const;
    int stderr() const;
    int id() const;
    void close();
private:
    int _stdin;
    int _stdout;
    int _stderr;
    pid_t _child_id;
};

class HttpObject : public InternetConnection
{
public:
    HttpObject(Internet* inet, const HttpAddress& addr, const string& post, const map<string, string>& header);
    int read(lua_State* lua);
    int response(lua_State* lua);
protected:
    bool update() override;
private:
    bool _response_ready;
    ValuePack _response;

    unique_ptr<Connection> _response_connection;
    PipedCommand _cmd;
};
