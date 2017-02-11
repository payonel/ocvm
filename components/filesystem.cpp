#include "filesystem.h"
#include "host.h"
#include "log.h"
#include "utils.h"
using std::streamsize;

Filesystem::Filesystem(Value& config, Host* host) :
    Component(config, host)
{
    // mkdir in host env path and filesystem address
    this->init(config.get(3).toString());

    add("open", &Filesystem::open);
    add("read", &Filesystem::read);
    add("close", &Filesystem::close);
    add("getLabel", &Filesystem::getLabel);
    add("list", &Filesystem::list);
    add("isDirectory", &Filesystem::isDirectory);
    add("exists", &Filesystem::exists);
}

void Filesystem::init(const string& loot)
{
    if (!utils::exists(path()))
    {
        utils::mkdir(path());
        if (!utils::copy(loot, path()))
        {
            lout << "filesystem failed to initialized, the source loot does not exist\n";
        }
    }
}

string Filesystem::clean(string arg, bool bAbs, bool removeEnd)
{
    size_t last = 0;
    bool hadEnd = false;
    while (true)
    {
        size_t len = arg.size();
        if (len == 0)
            break;

        size_t next = arg.find("/", last);

        if (next == string::npos)
            break;

        if (next == 0)
        {
            arg = arg.substr(1);
        }
        else if (next == len - 1)
        {
            arg = arg.substr(0, next);
            hadEnd = true;
        }
        else
        {
            last = next + 1;
        }
    }
    if (bAbs)
    {
        arg = "/" + arg;
    }
    if (hadEnd and !removeEnd)
    {
        arg = arg + "/";
    }

    return arg;
}

string Filesystem::relative(const string& requested, const string& full)
{
    string clean_requested = clean(requested, true, false);
    string clean_full = clean(full, true, false);
    if (clean_full.find(clean_requested) != 0)
    {
        lout << "error in expected relative truncation, could not find root path [";
        lout << clean_requested;
        lout << "] in [";
        lout << clean_full;
        lout << "]\n";
        return clean_full;
    }
    return clean_full.substr(clean_requested.size());
}

string Filesystem::path() const
{
    return clean(host()->envPath(), false, true) + clean(address(), true, true);
}

ValuePack Filesystem::open(const ValuePack& args)
{
    string filepath = Value::check(args, 0, "string").toString();
    string mode_text = Value::check(args, 1, "string", "nil").Or("r").toString();

    map<char, fstream::openmode> mode_map;
    mode_map['r'] = fstream::in;
    mode_map['w'] = fstream::out;
    mode_map['a'] = fstream::app;
    mode_map['b'] = fstream::binary;
    fstream::openmode mode;
    bool uninitialized = true;
    for (char c : mode_text)
    {
        auto it = mode_map.find(c);
        if (it == mode_map.end())
        {
            uninitialized = true;
            break;
        }
        if (uninitialized)
        {
            uninitialized = false;
            mode = it->second;
        }
        else
        {
            mode = mode | it->second;
        }
    }

    if (uninitialized || (mode & mode_map['r']) && (mode & mode_map['w']) || (mode & mode_map['a']) && (mode & mode_map['r']))
    {
        lout << "bad file mode: " << mode_text << endl;
        return ValuePack({Value::nil, filepath});
    }

    fstream* pf = new fstream;
    string fullpath = path() + clean(filepath, true, false);
    pf->open(fullpath, mode);

    if (!pf->is_open())
    {
        delete pf;
        return ValuePack({Value::nil, filepath});
    }
    // find next open handle index
    int index = 0;
    for (; index < _handles.size(); index++)
    {
        if (_handles.find(index) == _handles.end())
        {
            break;
        }
    }
    // either we found an open index and used break
    // OR the is sequentially full, but index is now beyond size
    // either way, index is available
    _handles[index] = pf;
    return ValuePack({index});
}

ValuePack Filesystem::read(const ValuePack& args)
{
    int index = (int)Value::check(args, 0, "number").toNumber(); // handle
    double dsize = Value::check(args, 1, "number").toNumber();
    static const streamsize max_read = (1024*1024*1024);
    streamsize size = (dsize > (double)max_read) ? max_read : static_cast<streamsize>(dsize);

    const auto& it = _handles.find(index);
    if (it == _handles.end())
    {
        return ValuePack({Value::nil, "bad file handle"});
    }
    fstream* fs = it->second;
    if (!fs->good())
        return ValuePack({Value::nil});

    string buffer;

    while (size > 0)
    {
        char c = fs->get();
        if (!fs->good())
        {
            break;
        }
        size--;
        buffer += c;
    }

    return ValuePack({buffer});
}

ValuePack Filesystem::close(const ValuePack& args)
{
    int index = (int)Value::check(args, 0, "number").toNumber(); // handle
    const auto& it = _handles.find(index);
    if (it == _handles.end())
    {
        return ValuePack({Value::nil, "bad file handle"});
    }
    fstream* fs = it->second;
    fs->close();
    _handles.erase(it);

    return ValuePack();
}

ValuePack Filesystem::getLabel(const ValuePack& args)
{
    return ValuePack {"label stub"};
}

ValuePack Filesystem::list(const ValuePack& args)
{
    string request_path = path() + clean(Value::check(args, 0, "string").toString(), true, false);
    auto listing = utils::list(request_path);

    ValuePack result {Value::table()};
    Value& t = result.at(0);
    for (const auto& item : listing)
    {
        string relative_item = clean(relative(request_path, item), false, false);
        t.insert(relative_item);
    }

    return result;
}

ValuePack Filesystem::isDirectory(const ValuePack& args)
{
    string relpath = clean(Value::check(args, 0, "string").toString(), true, true);
    ValuePack pack;
    pack.push_back( {utils::isDirectory(path() + relpath)} );
    return pack;
}

ValuePack Filesystem::exists(const ValuePack& args)
{
    return ValuePack { utils::exists(path() + clean(Value::check(args, 0, "string").toString(), true, true)) };
}
