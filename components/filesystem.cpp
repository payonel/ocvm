#include "filesystem.h"
#include "client.h"
#include "log.h"
#include "utils.h"
#include "apis/userdata.h"
#include <fstream>
#include <limits>
using namespace std;

class FileHandle : public UserData
{
public:
    FileHandle(Filesystem* fs, fstream* stream) :
        _fs(fs),
        _stream(stream)
    {
    }

    const Filesystem* fs() const { return _fs; }
    fstream* stream() const { return _stream; }

    void dispose() override
    {
        _fs->close(this);
    }
private:
    Filesystem* _fs;
    fstream* _stream;
};

Filesystem::Filesystem() :
    _isReadOnly(true),
    _tmpfs(false)
{
    add("open", &Filesystem::open);
    add("read", &Filesystem::read);
    add("write", &Filesystem::write);
    add("close", &Filesystem::close);
    add("getLabel", &Filesystem::getLabel);
    add("setLabel", &Filesystem::setLabel);
    add("list", &Filesystem::list);
    add("isDirectory", &Filesystem::isDirectory);
    add("exists", &Filesystem::exists);
    add("isReadOnly", &Filesystem::isReadOnly);
    add("seek", &Filesystem::seek);
    add("size", &Filesystem::size);
    add("lastModified" ,&Filesystem::lastModified);
    add("spaceUsed" ,&Filesystem::spaceUsed);
    add("spaceTotal" ,&Filesystem::spaceTotal);
    add("remove" ,&Filesystem::remove);
    add("makeDirectory", &Filesystem::makeDirectory);
    add("rename" ,&Filesystem::rename);
}

bool Filesystem::onInitialize()
{
    Value source_uri = config().get(ConfigIndex::SourceUri).Or(false);
    if (source_uri.type() == "string") // loot disk
    {
        _isReadOnly = true;
        _src = utils::proc_root() + source_uri.toString();
        _tmpfs = false;
        if (!utils::exists(_src))
        {
            lout << "loot disk not found: " << _src << endl;
            return false;
        }
    }
    else if (source_uri.type() == "boolean")
    {
        _isReadOnly = false;
        _src = "";
        _tmpfs = source_uri.toBool();
        // make local dir if it doesn't yet exist
        if (!utils::exists(path()))
        {
            utils::mkdir(path());
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
    if (_src.empty()) // use local path
        return clean(client()->envPath(), true, true) + clean(address(), true, true);
    else // else loot path
        return clean(_src, true, true);
}

string Filesystem::src() const
{
    return _src;
}

bool Filesystem::isReadOnly() const
{
    return _isReadOnly;
}

bool Filesystem::isTmpfs() const
{
    return _tmpfs;
}

int Filesystem::open(lua_State* lua)
{
    string filepath = Value::check(lua, 1, "string").toString();
    string mode_text = Value::check(lua, 2, "string", "nil").Or("r").toString();

    static map<char, fstream::openmode> mode_map
    {
        {'r', fstream::in},
        {'w', fstream::out},
        {'a', fstream::app},
        {'b', fstream::binary}
    };
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
    else if (isReadOnly() && (mode & (mode_map['w'] | mode_map['a'])))
    {
        return ValuePack::ret(lua, Value::nil, "filesystem is readonly");
    }

    fstream* pf = new fstream;
    string fullpath = path() + clean(filepath, true, false);

    pf->open(fullpath, mode);

    if (!pf->is_open())
    {
        delete pf;
        return ValuePack::ret(lua, Value::nil, filepath);
    }

    lua_settop(lua, 0);
    (void)create(lua, pf);
    return 1;
}

int Filesystem::read(lua_State* lua)
{
    fstream* fs = get_stream(lua);
    if (fs == nullptr) return 2;
    if (fs->eof()) return 0;

    double dsize = Value::check(lua, 2, "number").toNumber();
    static const streamsize max_read = (1024*1024*1024);
    streamsize size = (dsize > (double)max_read) ? max_read : static_cast<streamsize>(dsize);

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
    fstream* fs = get_stream(lua);
    if (fs == nullptr) return 2;

    string data = Value::check(lua, 2, "string").toString();

    (*fs) << data;

    return ValuePack::ret(lua, data.size());
}

int Filesystem::close(lua_State* lua)
{
    FileHandle* pfh = nullptr;
    (void)get_stream(lua, &pfh);
    if (pfh == nullptr) return 2;
    
    close(pfh);
    return 0;
}

int Filesystem::getLabel(lua_State* lua)
{
    return ValuePack::ret(lua, config().get(ConfigIndex::Label));
}

int Filesystem::setLabel(lua_State* lua)
{
    // labels can be readonly
    if (isTmpfs())
        return luaL_error(lua, "label is readonly");

    int stack = getLabel(lua);
    update(ConfigIndex::Label, Value::check(lua, 1, "string"));
    return stack;
}

int Filesystem::list(lua_State* lua)
{
    string request_path = path() + clean(Value::check(lua, 1, "string").toString(), true, false);
    auto listing = utils::list(request_path);

    Value t = Value::table();
    for (const auto& item : listing)
    {
        string relative_item = clean(relative(request_path, item), false, true);
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
    return ValuePack::ret(lua, isReadOnly());
}

int Filesystem::seek(lua_State* lua)
{
    fstream* fs = get_stream(lua);
    if (fs == nullptr) return 2;

    string whence = Value::check(lua, 2, "string").toString();    
    size_t to = (size_t)Value::check(lua, 3, "number", "nil").Or(0).toNumber();

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

int Filesystem::size(lua_State* lua)
{
    string filepath = Value::check(lua, 1, "string").toString();
    return ValuePack::ret(lua, utils::size(path() + clean(filepath, true, false)));
}

int Filesystem::lastModified(lua_State* lua)
{
    string filepath = Value::check(lua, 1, "string").toString();
    return ValuePack::ret(lua, utils::lastModified(path() + clean(filepath, true, false)));
}

fstream* Filesystem::get_stream(lua_State* lua, FileHandle** ppfh) const
{
    const Value& handle = Value::check(lua, 1, "userdata");
    FileHandle* pfh = reinterpret_cast<FileHandle*>(handle.toPointer());
    if (pfh && pfh->fs() == this)
    {
        fstream* fs = pfh->stream();
        if (fs->eof() || !fs->fail())
        {
            if (ppfh)
                *ppfh = pfh;
            return fs;
        }
    }
    ValuePack::ret(lua, Value::nil, "bad file handle");
    return nullptr;
}

int Filesystem::spaceUsed(lua_State* lua)
{
    return ValuePack::ret(lua, utils::size(path(), true));
}

int Filesystem::spaceTotal(lua_State* lua)
{
    return ValuePack::ret(lua, numeric_limits<double>::max());
}

int Filesystem::remove(lua_State* lua)
{
    string filepath = Value::check(lua, 1, "string").toString();
    filepath = path() + clean(filepath, true, false);
    if (isReadOnly())
    {
        return ValuePack::ret(lua, Value::nil, "filesystem is readonly");
    }
    else if (!utils::exists(filepath))
    {
        return ValuePack::ret(lua, Value::nil, "no such file or directory");
    }
    return ValuePack::ret(lua, utils::remove(filepath));
}

int Filesystem::makeDirectory(lua_State* lua)
{
    string dirpath = Value::check(lua, 1, "string").toString();
    dirpath = path() + clean(dirpath, true, false);
    if (isReadOnly())
    {
        return ValuePack::ret(lua, Value::nil, "filesystem is readonly");
    }
    else if (utils::exists(dirpath))
    {
        return ValuePack::ret(lua, false);
    }
    utils::mkdir(dirpath);
    return ValuePack::ret(lua, true);
}

int Filesystem::rename(lua_State* lua)
{
    string raw_from = Value::check(lua, 1, "string").toString();
    string raw_to = Value::check(lua, 2, "string").toString();

    string from = path() + clean(raw_from, true, true);
    string to = path() + clean(raw_to, true, true);
    if (isReadOnly())
    {
        return ValuePack::ret(lua, Value::nil, "filesystem is readonly");
    }
    else if (utils::exists(to))
    {
        return ValuePack::ret(lua, false);
    }
    else if (!utils::exists(from))
    {
        return ValuePack::ret(lua, Value::nil, raw_from);
    }
    return ValuePack::ret(lua, utils::rename(from, to));
}

void* Filesystem::create(lua_State* lua, fstream* pstream)
{
    // find next open handle index
    void* pAlloc = lua_newuserdata(lua, sizeof(FileHandle));
    FileHandle* pfh = new(pAlloc) FileHandle(this, pstream);
    _handles.insert(pfh);

    return pAlloc;
}

void Filesystem::close(FileHandle* pfh)
{
    if (pfh)
    {
        pfh->stream()->close();
        _handles.erase(pfh);
    }
}

