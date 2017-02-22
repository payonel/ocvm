#include "filesystem.h"
#include "client.h"
#include "log.h"
#include "utils.h"
using std::streamsize;

Filesystem::Filesystem()
{
    add("open", &Filesystem::open);
    add("read", &Filesystem::read);
    add("write", &Filesystem::write);
    add("close", &Filesystem::close);
    add("getLabel", &Filesystem::getLabel);
    add("list", &Filesystem::list);
    add("isDirectory", &Filesystem::isDirectory);
    add("exists", &Filesystem::exists);
    add("isReadOnly", &Filesystem::isReadOnly);
    add("seek", &Filesystem::seek);
}

bool Filesystem::onInitialize(Value& config)
{
    Value vloot = config.get(3).Or("");
    if (vloot.type() != "string")
    {
        lout << "invalid filesystem configuration: loot path not nil or string\n";
        return false;
    }
    _src = vloot.toString();
    if (!utils::exists(path()))
    {
        utils::mkdir(path());
        if (!_src.empty())
        {
            if (!utils::copy(_src, path()))
            {
                lout << "filesystem failed to initialized, the source loot does not exist\n";
                return false;
            }
        }
    }
    return true;
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
    return clean(client()->envPath(), false, true) + clean(address(), true, true);
}

string Filesystem::src() const
{
    return _src;
}

int Filesystem::open(lua_State* lua)
{
    string filepath = Value::check(lua, 1, "string").toString();
    string mode_text = Value::check(lua, 2, "string", "nil").Or("r").toString();

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
    
    if (uninitialized || 
        ((mode & mode_map['r']) && (mode & mode_map['w'])) || 
        ((mode & mode_map['a']) && (mode & mode_map['r'])))
    {
        lout << "bad file mode: " << mode_text << endl;
        return ValuePack::ret(lua, Value::nil, filepath);
    }

    fstream* pf = new fstream;
    string fullpath = path() + clean(filepath, true, false);
    pf->open(fullpath, mode);

    if (!pf->is_open())
    {
        delete pf;
        return ValuePack::ret(lua, Value::nil, filepath);
    }
    // find next open handle index
    size_t index = 0;
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
    return ValuePack::ret(lua, index);
}

int Filesystem::read(lua_State* lua)
{
    int index = (int)Value::check(lua, 1, "number").toNumber(); // handle
    double dsize = Value::check(lua, 2, "number").toNumber();
    static const streamsize max_read = (1024*1024*1024);
    streamsize size = (dsize > (double)max_read) ? max_read : static_cast<streamsize>(dsize);

    const auto& it = _handles.find(index);
    if (it == _handles.end())
    {
        return ValuePack::ret(lua, Value::nil, "bad file handle");
    }
    fstream* fs = it->second;
    if (!fs->good())
        return 0;

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

    return ValuePack::ret(lua, buffer);
}

int Filesystem::write(lua_State* lua)
{
    int index = (int)Value::check(lua, 1, "number").toNumber(); // handle
    string data = Value::check(lua, 2, "string").toString();

    const auto& it = _handles.find(index);
    if (it == _handles.end())
    {
        return ValuePack::ret(lua, Value::nil, "bad file handle");
    }

    fstream* fs = it->second;
    if (!fs->good())
    {
        return ValuePack::ret(lua, Value::nil, "bad file handle");
    }

    (*fs) << data;

    return ValuePack::ret(lua, data.size());
}

int Filesystem::close(lua_State* lua)
{
    int index = (int)Value::check(lua, 1, "number").toNumber(); // handle
    const auto& it = _handles.find(index);
    if (it == _handles.end())
    {
        return ValuePack::ret(lua, Value::nil, "bad file handle");
    }
    fstream* fs = it->second;
    fs->close();
    _handles.erase(it);

    return 0;
}

int Filesystem::getLabel(lua_State* lua)
{
    return ValuePack::ret(lua, "label stub");
}

int Filesystem::list(lua_State* lua)
{
    string request_path = path() + clean(Value::check(lua, 1, "string").toString(), true, false);
    auto listing = utils::list(request_path);

    Value t = Value::table();
    for (const auto& item : listing)
    {
        string relative_item = clean(relative(request_path, item), false, false);
        t.insert(relative_item);
    }

    return ValuePack::ret(lua, t);
}

int Filesystem::isDirectory(lua_State* lua)
{
    string relpath = clean(Value::check(lua, 1, "string").toString(), true, true);
    return ValuePack::ret(lua, utils::isDirectory(path() + relpath));
}

int Filesystem::exists(lua_State* lua)
{
    return ValuePack::ret(lua, utils::exists(path() + clean(Value::check(lua, 1, "string").toString(), true, true)));
}

int Filesystem::isReadOnly(lua_State* lua)
{
    return ValuePack::ret(lua, false);
}

int Filesystem::seek(lua_State* lua)
{
    int index = (int)Value::check(lua, 1, "number").toNumber(); // handle
    string whence = Value::check(lua, 2, "string").toString();    
    size_t to = (size_t)Value::check(lua, 3, "number", "nil").Or(0).toNumber();

    const auto& it = _handles.find(index);
    if (it == _handles.end())
    {
        return ValuePack::ret(lua, Value::nil, "bad file handle");
    }
    fstream* fs = it->second;
    if (!fs->good())
        return 0;

    std::ios_base::seekdir way;
    if (whence == "cur")
    {
        way = std::ios_base::cur;
    }
    else if (whence == "set")
    {
        way = std::ios_base::beg;
    }
    else if (whence == "end")
    {
        way = std::ios_base::end;
    }
    else
    {
        luaL_error(lua, "bad seek way");
        return 0;
    }

    fs->seekg(static_cast<fstream::pos_type>(to), way);
    return ValuePack::ret(lua, static_cast<double>(fs->tellg()));
}
