#pragma once

#include "io/event.h"
#include "worker.h"

#include <thread>

enum class ConnectionState
{
    Starting,
    Failed,
    Ready,
    Finished,
    Closed
};

class Connection
{
public:
    Connection(int id);
    Connection(const string& host, int system_port);
    virtual ~Connection();

    bool write(const vector<char>& vec);

    string label() const;
    ConnectionState state() const;
    ssize_t bytes_available() const;
    bool preload(ssize_t bytes);
    bool copy(vector<char>* pOut, ssize_t offset, ssize_t bytes);
    bool move(ssize_t bytes);
    bool can_read() const;
    bool can_write() const;
    void close();

    const static ssize_t max_buffer_size = 1024 * 16; // 16K, 8K is the max OC packet, double that for fun

protected:
    bool read(ssize_t bytes);

private:
    int _id = -1;
    string _host;
    int _port;

    ConnectionState _state;
    thread _connection_thread;

    bool _client_side = false;
    char _internal_buffer[max_buffer_size];
    ssize_t _buffer_size = 0;

    static void async_open(Connection* pc);
};

bool set_nonblocking(int id);
