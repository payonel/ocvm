#include "internet_drv.h"
#include "components/internet.h"

#include <algorithm>
#include <sstream>
using std::stringstream;

#include <unistd.h>
#include <sys/wait.h>
#include <string.h>
#include <fcntl.h>

#include "model/log.h"

////////////////// PIPED COMMAND ////////////////////////
PipedCommand::PipedCommand() :
    _stdin(-1),
    _stdout(-1),
    _stderr(-1),
    _child_id(-1)
{
}

PipedCommand::~PipedCommand()
{
    this->close();
}

bool PipedCommand::open(const string& command, const vector<string>& args)
{
    int pids[3][2] { {-1,-1}, {-1,-1}, {-1,-1} };
    for (int i = 0; i < 3; i++)
    {
        if (::pipe(pids[i]))
            return false;
    }

    _child_id = ::fork();
    if (_child_id == -1)
    {
        return false;
    }

    if (_child_id) // parent
    {
        _stdin = pids[STDIN_FILENO][1];
        ::close(pids[STDIN_FILENO][0]);
        set_nonblocking(_stdin);

        _stdout = pids[STDOUT_FILENO][0];
        ::close(pids[STDOUT_FILENO][1]);
        set_nonblocking(_stdout);

        _stderr = pids[STDERR_FILENO][0];
        ::close(pids[STDERR_FILENO][1]);
        set_nonblocking(_stderr);

        // success
        return true;
    }

    ::dup2(pids[STDIN_FILENO][0], STDIN_FILENO);
    ::close(pids[STDIN_FILENO][1]);

    ::dup2(pids[STDOUT_FILENO][1], STDOUT_FILENO);
    ::close(pids[STDOUT_FILENO][0]);

    ::dup2(pids[STDERR_FILENO][1], STDERR_FILENO);
    ::close(pids[STDERR_FILENO][0]);

    const char** vargs = new const char*[args.size() + 2];
    vargs[0] = command.data();
    for (size_t i = 0; i < args.size(); i++)
    {
        const auto& arg = args.at(i);
        vargs[i + 1] = arg.data();
    }
    vargs[args.size() + 1] = nullptr;

    ::execv(command.data(), const_cast<char*const*>(vargs));
    delete [] vargs;
    ::_exit(EXIT_FAILURE);
    return false;
}

int PipedCommand::stdin() const
{
    return _stdin;
}

int PipedCommand::stdout() const
{
    return _stdout;
}

int PipedCommand::stderr() const
{
    return _stderr;
}

int PipedCommand::id() const
{
    return _child_id;
}

void PipedCommand::close()
{
    ::close(_child_id);
    ::close(_stdin);
    ::close(_stdout);
    ::close(_stderr);
}

/////////////////////////////////////////////////////////

static string trim(const string& value)
{
    size_t first = value.find_first_not_of(" \t\n\r");
    if (first == string::npos)
        return value;
    string tvalue = value.substr(first);
    size_t last = tvalue.find_last_not_of(" \t\n\r");
    if (last != string::npos)
        tvalue = tvalue.substr(0, last + 1);

    return tvalue;
}

static string escape(const string& text)
{
    string result;

    size_t last = 0;
    while (true)
    {
        size_t next = text.find("'", last);
        result += text.substr(last, next);
        if (next == string::npos)
        {
            break;
        }
        result += "'\\''";
        last = next + 1;
    }

    return "'" + result + "'";
}

InternetConnection::InternetConnection(Internet* inet) :
    _inet(inet),
    _connection(),
    _needs_connection(false),
    _needs_data(false)
{
    add("finishConnect", &InternetConnection::finishConnect);
    add("read", &InternetConnection::read);
    add("close", &InternetConnection::close);
}

void InternetConnection::dispose()
{
    _close();
    _inet->release(this);
}

int InternetConnection::finishConnect(lua_State* lua)
{
    _needs_connection = _connection->state() < ConnectionState::Ready;
    return ValuePack::ret(lua, !_needs_connection);
}

int InternetConnection::close(lua_State* lua)
{
    _close();
    return 0;
}

void InternetConnection::_close()
{
    _connection->close();
}

bool InternetConnection::update()
{
    bool updated = false;
    if (_needs_connection)
    {
        if (_connection->state() >= ConnectionState::Ready)
        {
            _needs_connection = false;
            updated = true;
        }
    }
    if (_needs_data)
    {
        if (_connection->state() == ConnectionState::Ready)
        {
            auto avail = _connection->bytes_available();
            if (_connection->preload(avail + 1))
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
    if (!_connection->can_read())
        return ValuePack::ret(lua, Value::nil, "not connected");

    vector<char> buffer;
    LUA_NUMBER default_n = INT_MAX;
    ssize_t n = static_cast<ssize_t>(Value::checkArg<LUA_NUMBER>(lua, 1, &default_n));

    _connection->preload(n);
    n = std::min(n, _connection->bytes_available());
    _connection->copy(&buffer, 0, n);
    _connection->move(buffer.size());

    if (buffer.size() == 0 && _connection->state() == ConnectionState::Finished)
    {
        _connection->close();
        return ValuePack::ret(lua, Value::nil);
    }

    return ValuePack::ret(lua, buffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

TcpObject::TcpObject(Internet* inet, const string& addr, int port) :
    InternetConnection(inet)
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


/////////////////////////////////////////////////////////////////////////////////////////////////

HttpObject::HttpObject(Internet* inet, const HttpAddress& addr, const string& post, const map<string, string>& header) :
    InternetConnection(inet),//, addr.hostname, addr.port),
    _response_ready(false)
{
    add("response", &HttpObject::response);

    this->name("HttpObject");

    vector<string> args {"-q", "-S", "--no-check-certificate", "-O", "/proc/self/fd/1"};
    if (post.size())
    {
        args.push_back("--post-data=" + escape(post));
        args.push_back("--method=POST");
    }
    for (const auto& header_pair : header)
    {
        args.push_back("--header=" + escape(header_pair.first + ": " + header_pair.second));
    }
    if (!header.empty() && header.find("Host") == header.end())
    {
        args.push_back("--header=" + escape("Host: " + addr.hostname));
    }
    if (addr.valid)
    {
        args.push_back(addr.raw);
        _cmd.open("/usr/bin/wget", args);
    }

    _connection.reset(new Connection(_cmd.stdout()));
    _response_connection.reset(new Connection(_cmd.stderr()));
}

int HttpObject::response(lua_State* lua)
{
    if (!_response_ready)
        return ValuePack::ret(lua, Value::nil);

    return _response.push(lua);
}

bool HttpObject::update()
{
    if (!_response_ready)
    {
        if (!_response_connection->can_read())
        {
            _response_ready = true;
            _response.clear();
            _response.push_back(Value::nil);
            _response.push_back("connectioned closed");
        }
        else
        {
            bool response_finished = _response_connection->state() == ConnectionState::Finished;
            size_t prev_buffer_size = _response_connection->bytes_available();
            _response_connection->preload(prev_buffer_size + 128);
            size_t buffer_size = _response_connection->bytes_available();

            _response_ready = response_finished && prev_buffer_size == buffer_size;

            vector<char> buffer;
            _response_connection->copy(&buffer, 0, buffer_size);
            size_t from = 0;

            while (from < buffer.size())
            {
                static const vector<char> nl { '\n' };
                auto line_iterator = std::search(std::begin(buffer) + from, std::end(buffer), std::begin(nl), std::end(nl));

                if (line_iterator == buffer.end())
                {
                    break;
                }

                stringstream ss;
                string line(buffer.begin() + from, line_iterator);
                _response_connection->move(line.size() + 1);
                from = (line_iterator - buffer.begin()) + 1;

                ss << line;

                string key;
                string value;
                if (_response.size() == 0)
                {
                    // HTTP/1.1 200 OK
                    ss >> key;
                    key = trim(key);
                    int code;
                    string message;
                    ss >> code;
                    std::getline(ss, message);
                    if (!ss || !ss.eof())
                    {
                        // failed to parse
                        _response_ready = true;
                        _response.push_back(Value::nil);
                        _response.push_back("failed to parse http response");
                        _response_connection->close();
                        break;
                    }

                    message = trim(message);
                    _response.push_back(code);
                    _response.push_back(message);
                }
                else
                {
                    //    Data: Mon, 19 May 2014 12:46:36 GMT
                    std::getline(ss, key, ':');
                    key = trim(key);
                    std::getline(ss, value);
                    value = trim(value);
                    if (_response.size() < 3)
                    {
                        _response.push_back(Value::table());
                    }
                    _response.at(2).set(key, value);
                }
            }
        }
    }

    return InternetConnection::update();
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

