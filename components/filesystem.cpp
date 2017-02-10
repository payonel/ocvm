#include "filesystem.h"
#include "host.h"
#include "log.h"
#include "utils.h"
using std::streamsize;

Filesystem::Filesystem(const string& type, const Value& init, Host* host) :
    Component(type, init, host)
{
    // mkdir in host env path and filesystem address
    this->init(init.get(2).toString());

    add("open", &Filesystem::open);
    add("read", &Filesystem::read);
    add("close", &Filesystem::close);
    add("getLabel", &Filesystem::getLabel);
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

string Filesystem::path() const
{
    return host()->envPath() + "/" + address();
}

ValuePack Filesystem::open(const ValuePack& args)
{
    string filepath = Value::check(args, 0, "string").toString();
    Value vmode = Value::check(args, 1, "string", "nil");
    string mode_text = vmode ? vmode.toString() : "r";

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
    pf->open(path() + filepath, mode);

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
