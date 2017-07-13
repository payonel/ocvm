#include "modem_drv.h"
#include "model/log.h"

// c includes for sockets
#include <sys/file.h>
#include <sys/socket.h>
#include <unistd.h>
#include <netdb.h>

unique_ptr<FileLock> FileLock::create(const string& path)
{
    // we can only attempt to create the server if we only the file lock
    int server_pool_file = ::open("/tmp/ocvm_modem.lock", O_CREAT | O_TRUNC);
    if (::flock(server_pool_file, LOCK_EX | LOCK_NB) == -1)
    {
        lout << "flock denied\n";
        ::close(server_pool_file);
        return nullptr; //denied
    }
    
    lout << "flock acquired\n";
    return unique_ptr<FileLock>(new FileLock(server_pool_file));
}

FileLock::FileLock(int fd) :
    _fd(fd)
{
}

FileLock::~FileLock()
{
    ::flock(_fd, LOCK_UN);
    ::close(_fd);
}

ServerPool::ServerPool(int id, unique_ptr<FileLock> lock) :
    _id(id),
    _lock(std::move(lock))
{
}

ServerPool::~ServerPool()
{
    lout << "modem ServerPool shutdown\n";
    ::close(_id);
}

bool ServerPool::onStart()
{
    lout << "modem ServerPool starting\n";
    return true;
}

bool ServerPool::remove(int id)
{
    const auto& conn_it = _connections.find(id);
    if (conn_it == _connections.end())
        return false;

    lout << "modem ServerPool removed client: " << id << endl;
    conn_it->second->stop();
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
                lout << "server accept failed\n";
                return false;
            }
            break;
        }

        work.set();

        if (!set_nonblocking(client_socket))
        {
            lout << "modem ServerPool accepted client socket but failed to set non blocking\n";
            ::close(client_socket);
        }
        else
        {
            auto client = new Connection(client_socket);
            _connections[client_socket] = client;
            client->start();
        }
    }

    // now rebroadcast all the messages
    vector<int> ids;
    vector<int> removals;
    for (auto pair : _connections)
    {
        if (pair.second->isRunning())
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

        ModemEvent me;
        while (conn->pop(me))
        {
            work.set();
            // rebroadcast
            for (auto other_id : ids)
            {
                if (other_id == id)
                    continue; // skip self

                Connection* sib = _connections.at(other_id);
                sib->write(MessageType::RelayMessage, me.payload);
            }
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

    lout << "modem ServerPool thread shutdown\n";
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
        lout.write("modem failed: getaddrinfo error ", gai_strerror(status));
        return nullptr;
    }

    int id = -1;

    for (auto pServer = server_info; pServer; pServer = pServer->ai_next)
    {
        int yes = 1;
        if ((id = ::socket(pServer->ai_family, pServer->ai_socktype, pServer->ai_protocol)) == -1)
        {
            lout << "modem failed: bad socket\n";
        }
        else if (setsockopt(id, SOL_SOCKET, SO_REUSEPORT, &yes, sizeof(yes)) == -1)
        {
            lout << "modem ServerPool socket created but failed to switch to reuse\n";
        }
        else if (::bind(id, pServer->ai_addr, pServer->ai_addrlen) == -1)
        {
            if (errno != 98)
            {
                lout.write("modem ServerPool failed to bind: ", errno);
            }
        }
        // bind succeeded, so listen better!
        else if (::listen(id, 20) == -1)
        {
            lout << "modem was able to bind, but then failed to listen\n";
        }
        else if (!set_nonblocking(id))
        {
            lout << "modem was able to listen, but then failed to switch to nonblocking\n";
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
        lout << "server failed to start";
        return nullptr;
    }

    return server;
}

ModemDriver::ModemDriver(EventSource<ModemEvent>* source, int system_port) :
    _source(source),
    _system_port(system_port)
{
}

bool ModemDriver::send(const vector<char>& payload)
{
    // runOnce can reset the connection, lock it
    make_lock();
    if (!connected())
    {
        lout << "modem::send failed: not connected\n";
        return false;
    }

    return _connection->write(MessageType::SourceMessage, payload);
}

bool ModemDriver::onStart()
{
    return true;
}

bool ModemDriver::runOnce()
{
    NiceWork work;
    if (!connected())
    {
        if (_connection)
        {
            _connection->stop();
            _connection.reset(nullptr);
        }

        _connection = Connection::create("127.0.0.1", _system_port);

        if (!_connection) // if we still don't have a connection, perhaps we need a local server
        {
            lout << "modem: no remote server detected\n";
            if (!_local_server)
            {
                _local_server = ServerPool::create(_system_port);
            }
            return true; // try connection again on next thread loop
        }
    }

    ModemEvent me;
    while (_connection->pop(me))
    {
        work.set();
        _source->push(me);
    }

    return true;
}

void ModemDriver::onStop()
{
    if (_connection)
    {
        _connection->stop();
    }
    _connection.reset(nullptr);
    if (_local_server)
    {
        _local_server->stop();
    }
    _local_server.reset(nullptr);
    lout << "modem shutdown\n";
}

bool ModemDriver::connected() const
{
    return _connection && _connection->ok();
}
