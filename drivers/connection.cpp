#include "connection.h"
#include "model/log.h"

// c includes for sockets
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>

Connection::Connection(int id) :
    _id(id)
{
}

Connection::~Connection()
{
    ::close(_id);
}

bool Connection::onStart()
{
    lout.write(label(), " thread starting");
    return true;
}

bool Connection::read(ssize_t bytes)
{
    while (_buffer_size < bytes)
    {
        ssize_t bytes_received = ::read(_id, _internal_buffer + _buffer_size, bytes - _buffer_size);
        if (bytes_received <= 0) // not ready or closed or failed or interrupted
        {
            if (bytes_received == 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR))
            {
                if (errno == 11)
                    lout.write(label(), " disconnected");
                else
                    lout.write(label(), " read failed: ", bytes_received, " errno:", errno);
                
                _failed = true;
            }
            return false;
        }
        _buffer_size += bytes_received;
    }
    return true;
}

bool Connection::runOnce()
{
    NiceWork work;
    if (_failed)
        return false;

    // read next packet from server
    const ssize_t header_size = sizeof(int32_t) + sizeof(char);
    if (!read(header_size))
        return true;

    int32_t packet_size = 0;
    char* p = reinterpret_cast<char*>(&packet_size);
    p[0] = _internal_buffer[0];
    p[1] = _internal_buffer[1];
    p[2] = _internal_buffer[2];
    p[3] = _internal_buffer[3];
    MessageType msg = static_cast<MessageType>(_internal_buffer[4]);

    if (msg != MessageType::RelayMessage && msg != MessageType::SourceMessage)
    {
        lout.write(label(), " invalid packet: ", static_cast<int>(msg));
        return true;
    }

    ssize_t end = header_size + packet_size;

    if (max_buffer_size < end)
    {
        lout.write(label(), " likely bad packet, size reported: ", packet_size);
        return true;
    }

    if (!read(end))
        return true;

    // we have the entire packet!
    work.set();
    ModemEvent me;
    std::copy(_internal_buffer + header_size, _internal_buffer + end,
        std::back_inserter(me.payload));
    ::memmove(_internal_buffer, _internal_buffer + end, _buffer_size - end);
    lout.write(label(), " packet completed: ", _buffer_size, " bytes");
    _buffer_size = 0;
    this->push(me);

    return true;
}

void Connection::onStop()
{
}

bool Connection::write(MessageType msg, const vector<char>& payload)
{
    make_lock();
    if (!isRunning() || !ok())
    {
        return false;
    }

    if (msg == MessageType::SourceMessage)
    {
        this->push({payload});
    }

    char header[5];
    int32_t data_size = payload.size();
    char* p = reinterpret_cast<char*>(&data_size);
    header[0] = p[0];
    header[1] = p[1];
    header[2] = p[2];
    header[3] = p[3];
    header[4] = static_cast<char>(msg);
    lout.write(label(), " writing packet to network: ", data_size+5, " bytes");
    return
        ::write(_id, header, sizeof(header)) != -1 &&
        ::write(_id, payload.data(), payload.size()) != -1;
}

string Connection::label() const
{
    stringstream ss;
    if (_client_side)
        ss << "Client->Server ";
    else
        ss << "Server->Client ";
    ss << "Connection[" << _id << "]";
    return ss.str();
}

unique_ptr<Connection> Connection::create(const string& host, int system_port)
{
    int status;
    addrinfo hints {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    addrinfo* server_info = nullptr;

    string port_text;
    {
        stringstream ss;
        ss << system_port;
        port_text = ss.str();
    }

    if ((status = ::getaddrinfo(host.c_str(), port_text.c_str(), &hints, &server_info)) != 0)
    {
        lout << "modem failed: getaddrinfo error " << gai_strerror(status) << endl;
        return nullptr;
    }

    int id = -1;

    // now connect to the server
    for (auto pServer = server_info; pServer; pServer = pServer->ai_next)
    {
        if ((id = ::socket(pServer->ai_family, pServer->ai_socktype, pServer->ai_protocol)) == -1)
        {
            lout << "modem failed: bad socket\n";
        }
        else if (::connect(id, pServer->ai_addr, pServer->ai_addrlen) == -1)
        {
        }
        else if (!set_nonblocking(id))
        {
            lout << "failed to switch socket to non blocking\n";
        }
        else
        {
            break;
        }

        ::close(id);
        id = -1;
    }

    freeaddrinfo(server_info);

    if (id == -1)
        return nullptr;

    unique_ptr<Connection> result(new Connection(id));
    result->_client_side = true;
    if (!result->start())
    {
        return nullptr;
    }

    return result;
}

bool Connection::ok() const
{
    return _id != -1 && !_failed;
}

bool set_nonblocking(int id)
{
    int flags;
    if ((flags = fcntl(id, F_GETFL, 0)) < 0) 
    {
        return false;
    }

    if (fcntl(id, F_SETFL, flags | O_NONBLOCK) < 0) 
    {
        return false;
    }

    return true;
}
