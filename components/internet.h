#pragma once
#include "component.h"
#include "model/value.h"

#include "drivers/connection.h"

#include <set>
using std::set;

class InternetConnection;
class Internet : public Component
{
public:
    enum ConfigIndex
    {
        TcpEnabled = Component::ConfigIndex::Next,
        HttpEnabled,
    };

    int isTcpEnabled(lua_State*);
    int isHttpEnabled(lua_State*);
    int connect(lua_State*);
    int request(lua_State*);

    bool release(InternetConnection* pConn);
    void monitor_connection(InternetConnection* inet);
    void monitor_data(InternetConnection* inet);

    Internet();
    ~Internet();
protected:
    bool onInitialize() override;
    RunState update() override;

    bool parsePort(string* pAddr, int* pPort) const;
private:
    bool _tcp;
    bool _http;

    set<InternetConnection*> _connections;
};
