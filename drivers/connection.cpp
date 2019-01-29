#include "connection.h"

// c includes for sockets
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>

#include <sstream>

using std::vector;
using std::string;
using std::stringstream;

void Connection::async_open(Connection* pc)
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
        ss << pc->_port;
        port_text = ss.str();
    }

    if ((status = ::getaddrinfo(pc->_host.c_str(), port_text.c_str(), &hints, &server_info)) != 0)
    {
        pc->_state = ConnectionState::Failed;
        return;
    }

    int id = -1;

    // now connect to the server
    for (auto pServer = server_info; pServer; pServer = pServer->ai_next)
    {
        if ((id = ::socket(pServer->ai_family, pServer->ai_socktype, pServer->ai_protocol)) == -1)
        {
            // modem failed: bad socket
        }
        else if (::connect(id, pServer->ai_addr, pServer->ai_addrlen) == -1)
        {
        }
        else if (!set_nonblocking(id))
        {
            // failed to switch socket to non blocking
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
    {
        pc->_state = ConnectionState::Failed;
        return;
    }

    pc->_id = id;
    pc->_client_side = true;
    pc->_state = ConnectionState::Ready;
}

Connection::Connection(int id) :
    _id(id)
    ,_host("")
    ,_port(-1)
    ,_state(ConnectionState::Ready)
    //,_connection_thread,
{
}

Connection::Connection(const string& host, int system_port) :
    _id(-1)
    ,_host(host)
    ,_port(system_port)
    ,_state(ConnectionState::Starting)
    ,_connection_thread(Connection::async_open, this)
{
}

void Connection::close()
{
    if (_connection_thread.joinable())
        _connection_thread.join();
    ::close(_id);
    _state = ConnectionState::Closed;
}

Connection::~Connection()
{
    close();
}

bool Connection::write(const vector<char>& vec)
{
    if (state() != ConnectionState::Ready)
        return false;

    return ::send(_id, vec.data(), vec.size(), MSG_NOSIGNAL) != -1;
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

ConnectionState Connection::state() const
{
    return _state;
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

ssize_t Connection::bytes_available() const
{
    return _buffer_size;
}

bool Connection::preload(ssize_t bytes)
{
    if (_state != ConnectionState::Ready)
        return bytes <= _buffer_size;

    if (bytes > Connection::max_buffer_size)
    {
        if (_buffer_size == Connection::max_buffer_size)
        {
            // buffer overflow, cannot read more from socket
            _state = ConnectionState::Failed;
            return false;
        }
        bytes = Connection::max_buffer_size;
    }

    while (_buffer_size < bytes)
    {
        ssize_t bytes_received = ::read(_id, _internal_buffer + _buffer_size, bytes - _buffer_size);
        if (bytes_received <= 0) // not ready or closed or failed or interrupted
        {
            if (bytes_received == 0 || (errno != EAGAIN && errno != EWOULDBLOCK && errno != EINTR))
            {
                // if (errno == 11)
                    // disconnected
                // else
                    // read failed

                _state = ConnectionState::Finished;

                ::close(_id);
                _id = -1;
            }
            return false;
        }
        _buffer_size += bytes_received;
    }
    return true;
}

bool Connection::back_insert(vector<char>* pOut, ssize_t offset, ssize_t bytes)
{
    if (!preload(offset + bytes))
        return false;

    std::copy(_internal_buffer + offset, _internal_buffer + offset + bytes, std::back_inserter(*pOut));
    return true;
}

bool Connection::move(ssize_t bytes)
{
    if (_buffer_size < bytes)
        return false;

    if (_buffer_size > bytes)
        ::memmove(_internal_buffer, _internal_buffer + bytes, _buffer_size - bytes);

    _buffer_size -= bytes;

    return true;
}

bool Connection::can_read() const
{
    return _state == ConnectionState::Ready || _state == ConnectionState::Finished;
}

bool Connection::can_write() const
{
    return _state == ConnectionState::Ready;
}

bool Connection::readyNextPacket(std::vector<char>* buffer, bool keepPacketSize)
{
    buffer->clear();

    // read next packet from server
    constexpr ssize_t header_size = sizeof(int32_t);
    if (!back_insert(buffer, 0, header_size))
        return false;

    int32_t packet_size = 0;
    char* p = reinterpret_cast<char*>(&packet_size);
    p[0] = buffer->at(0);
    p[1] = buffer->at(1);
    p[2] = buffer->at(2);
    p[3] = buffer->at(3);

    ssize_t end = header_size + packet_size;
    if (Connection::max_buffer_size < end)
    {
        // modem likely bad packet, size reported: packet_size
        move(header_size);
        return false;
    }

    if (!keepPacketSize)
    {
        buffer->clear();
    }

    if (!back_insert(buffer, header_size, packet_size))
        return false;

    move(end);

    // modem packet completed: ${end} bytes
    return true;
}
