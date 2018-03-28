#include "modem_drv.h"
#include "model/log.h"

// c includes for sockets
#include <sys/file.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

bool readyNextPacket(Connection* conn, vector<char>* buffer, bool keepPacketSize)
{
    if (!conn)
        return false;

    buffer->clear();

    // read next packet from server
    constexpr ssize_t header_size = sizeof(int32_t);
    if (!conn->back_insert(buffer, 0, header_size))
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
        conn->move(header_size);
        return false;
    }

    if (!keepPacketSize)
    {
        buffer->clear();
    }

    if (!conn->back_insert(buffer, header_size, packet_size))
        return false;

    conn->move(end);

    // modem packet completed: ${end} bytes
    return true;
}

bool ModemDriver::readNextModemMessage(ModemEvent& mev)
{
    return readyNextPacket(_connection.get(), &mev.payload, false);
}

unique_ptr<FileLock> FileLock::create(const string& path)
{
    // we can only attempt to create the server if we only the file lock
    int server_pool_file = ::open(path.c_str(), O_CREAT | O_TRUNC);
    if (::flock(server_pool_file, LOCK_EX | LOCK_NB) == -1)
    {
        // flock denied
        ::close(server_pool_file);
        return nullptr; //denied
    }
    
    // flock acquired
    return unique_ptr<FileLock>(new FileLock(path, server_pool_file));
}

FileLock::FileLock(const string& path, int fd) :
    _path(path),
    _fd(fd)
{
}

FileLock::~FileLock()
{
    ::flock(_fd, LOCK_UN);
    ::close(_fd);
    
    ::unlink(_path.c_str());
}

ServerPool::ServerPool(int id, unique_ptr<FileLock> lock) :
    _id(id),
    _lock(std::move(lock))
{
}

ServerPool::~ServerPool()
{
    ::close(_id);
}

bool ServerPool::onStart()
{
    return true;
}

bool ServerPool::remove(int id)
{
    const auto& conn_it = _connections.find(id);
    if (conn_it == _connections.end())
        return false;

    delete conn_it->second;
    _connections.erase(conn_it);
    return true;
}

bool ServerPool::runOnce()
{
    // accept new connections
    NiceWork work;
    while (true)
    {
        sockaddr_storage client_addr {};
        socklen_t addr_size {};
        int client_socket = ::accept(_id, reinterpret_cast<sockaddr*>(&client_addr), &addr_size);

        if (client_socket <= 0)
        {
            if (errno != EAGAIN && errno != EWOULDBLOCK)
            {
                return false;
            }
            break;
        }

        work.set();

        if (!set_nonblocking(client_socket))
        {
            // modem ServerPool accepted client socket but failed to set non blocking
            ::close(client_socket);
        }
        else
        {
            auto client = new Connection(client_socket);
            _connections[client_socket] = client;
        }
    }

    // now rebroadcast all the messages
    vector<int> ids;
    vector<int> removals;
    for (auto pair : _connections)
    {
        if (pair.second->state() == ConnectionState::Ready)
            ids.push_back(pair.first);
        else
            removals.push_back(pair.first);
    }

    for (auto id : removals)
    {
        remove(id);
        work.set();
    }

    for (auto id : ids)
    {
        Connection* conn = _connections.at(id);

        vector<char> buffer;
        if (!readyNextPacket(conn, &buffer, true))
            continue;

        work.set();
        // rebroadcast
        for (auto other_id : ids)
        {
            Connection* sib = _connections.at(other_id);
            sib->write(buffer);
        }
    }

    return true;
}

void ServerPool::onStop()
{
    vector<int> copy;
    for (auto pair : _connections)
    {
        copy.push_back(pair.first);
    }

    for (auto id : copy)
    {
        remove(id);
    }
}

unique_ptr<ServerPool> ServerPool::create(int system_port)
{
    unique_ptr<FileLock> lock = FileLock::create("/tmp/ocvm.modem.lock");
    if (!lock)
        return nullptr;

    // we assume there is a super modem in this network
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

    if ((status = ::getaddrinfo(nullptr, port_text.c_str(), &hints, &server_info)) != 0)
    {
        // modem failed: getaddrinfo error
        return nullptr;
    }

    int id = -1;

    for (auto pServer = server_info; pServer; pServer = pServer->ai_next)
    {
        int yes = 1;
        if ((id = ::socket(pServer->ai_family, pServer->ai_socktype, pServer->ai_protocol)) == -1)
        {
            // modem failed: bad socket
        }
        else if (setsockopt(id, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) == -1)
        {
            // modem ServerPool socket created but failed to switch to reuse
        }
        else if (::bind(id, pServer->ai_addr, pServer->ai_addrlen) == -1)
        {
            // if (errno != 98)
            // {
                // modem ServerPool failed to bind
            // }
        }
        // bind succeeded, so listen better!
        else if (::listen(id, 20) == -1)
        {
            // modem was able to bind, but then failed to listen
        }
        else if (!set_nonblocking(id))
        {
            // modem was able to listen, but then failed to switch to nonblocking
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
        return nullptr;
    }

    unique_ptr<ServerPool> server(new ServerPool(id, std::move(lock)));
    if (!server->start())
    {
        // server failed to start
        return nullptr;
    }

    return server;
}

ModemDriver::ModemDriver(EventSource<ModemEvent>* source, int system_port, const std::string& system_address) :
    _source(source),
    _system_port(system_port),
    _system_address(system_address)
{
}

bool ModemDriver::send(const vector<char>& payload)
{
    // runOnce can reset the connection, lock it
    auto lock = make_lock();
    if (!_connection || !_connection->can_write())
    {
        // modem::send failed: not connected
        return false;
    }

    int32_t data_size = static_cast<int32_t>(payload.size());
    char* p = reinterpret_cast<char*>(&data_size);
    vector<char> header { p[0], p[1], p[2], p[3] };
    if (!_connection->write(header))
        return false;
    return _connection->write(payload);
}

bool ModemDriver::onStart()
{
    return true;
}

bool ModemDriver::runOnce()
{
    NiceWork work;
    if (!_connection)
    {
        _connection.reset(new Connection(_system_address, _system_port));
    }
    switch (_connection->state())
    {
    case ConnectionState::Starting:
        // still waiting for connection to complete
        return true;
    case ConnectionState::Failed:
        // no server
        _connection.reset(nullptr);
        if (!_local_server)
        {
            _local_server = ServerPool::create(_system_port);
        }
        return true;
    case ConnectionState::Ready:
        break;
    case ConnectionState::Finished:
    case ConnectionState::Closed:
        // the server may have closed this connection
        _connection.reset(nullptr);
        return true;
    }

    ModemEvent me;
    while (readNextModemMessage(me))
    {
        work.set();
        _source->push(me);
    }

    return true;
}

void ModemDriver::onStop()
{
    _connection.reset(nullptr);
    if (_local_server)
    {
        _local_server->stop();
    }
    _local_server.reset(nullptr);
    // modem shutdown
}
