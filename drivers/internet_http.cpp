#include "internet_http.h"
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

#include <sstream>
using std::stringstream;

static bool SetHttpGenerator()
{
  InternetConnection::HttpGenRegistry& http_gen = InternetConnection::http_gen();
  http_gen = [] (UserDataAllocator allocator, const HttpConstructionParameters& args) {
    return new(allocator(sizeof(HttpObject))) HttpObject(args.addr, args.post, args.header);
  };
  return true;
}

bool HttpObject::s_registered = SetHttpGenerator();

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

////////////////// PIPED COMMAND ////////////////////////

PipedCommand::PipedCommand() :
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
        int stdin = pids[STDIN_FILENO][1];
        ::close(pids[STDIN_FILENO][0]);
        set_nonblocking(stdin);

        int stdout = pids[STDOUT_FILENO][0];
        ::close(pids[STDOUT_FILENO][1]);
        set_nonblocking(stdout);

        int stderr = pids[STDERR_FILENO][0];
        ::close(pids[STDERR_FILENO][1]);
        set_nonblocking(stderr);

        _stdin.reset(new Connection(stdin));
        _stdout.reset(new Connection(stdout));
        _stderr.reset(new Connection(stderr));

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

Connection* PipedCommand::stdin() const
{
    return _stdin.get();
}

Connection* PipedCommand::stdout() const
{
    return _stdout.get();
}

Connection* PipedCommand::stderr() const
{
    return _stderr.get();
}

int PipedCommand::id() const
{
    return _child_id;
}

void PipedCommand::close()
{
    ::close(_child_id);
}

/////////////////////////////////////////////////////////////////////////////////////////////////

HttpObject::HttpObject(const HttpAddress& addr, const string& post, const map<string, string>& header) :
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
        // http request: addr.raw
        _cmd.open("/usr/bin/wget", args);
    }
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
        Connection* resp = _cmd.stderr();
        if (!resp->can_read())
        {
            _response_ready = true;
            _response.clear();
            _response.push_back(Value::nil);
            _response.push_back("connectioned closed");
        }
        else
        {
            bool response_finished = resp->state() == ConnectionState::Finished;
            size_t prev_buffer_size = resp->bytes_available();
            resp->preload(prev_buffer_size + 128);
            size_t buffer_size = resp->bytes_available();

            _response_ready = response_finished && prev_buffer_size == buffer_size;

            vector<char> buffer;
            resp->back_insert(&buffer, 0, buffer_size);
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
                resp->move(line.size() + 1);
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
                        resp->close();
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

Connection* HttpObject::connection() const
{
    return _cmd.stdout();
}

