#pragma once

#include "io/event.h"
#include "worker.h"

#include <memory>
using std::unique_ptr;

enum class MessageType
{
    SourceMessage,
    RelayMessage
};

class Connection : public Worker, public EventSource<ModemEvent>
{
public:
    Connection(int id);
    virtual ~Connection();
    static unique_ptr<Connection> create(const string& host, int system_port);

    bool write(MessageType type, const vector<char>& payload);

    string label() const;
    bool ok() const;

protected:
    bool onStart() override;
    bool runOnce() override;
    void onStop() override;

    bool read(ssize_t bytes);

private:
    int _id = -1;
    bool _client_side = false;
    bool _failed = false;

    const static ssize_t max_buffer_size = 1024 * 16; // 16K, 8K is the max OC packet, double that for fun
    char _internal_buffer[max_buffer_size];
    ssize_t _buffer_size = 0;
};

bool set_nonblocking(int id);
