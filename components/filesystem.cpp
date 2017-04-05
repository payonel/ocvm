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
    FileHandle(Filesystem* fs) : _fs(fs)
    {
    }

    const Filesystem* fs() const { return _fs; }

    void dispose() override
    {
        close();
        _fs->release(this);
    }

    virtual bool isOpen() const
    {
        if (this == nullptr || _fs == nullptr)
            return false;

        return _isOpen;
    }

    bool canRead() const
    {
        return isOpen() && _isReader();
    }

    bool canWrite() const
    {
        return isOpen() && !_isReader();
    }

    void close()
    {
        _close();
        _isOpen = false;
    }

    virtual vector<char> read(streamsize size) {return {};}
    virtual void write(const vector<char>& data) {}
    virtual bool eof() const { return false; }

    virtual bool seek(int32_t to, std::ios_base::seekdir way) = 0;
    virtual int32_t tell() = 0;

protected:
    virtual void _close() = 0;
    virtual bool _isReader() const = 0;
    bool _isOpen;

private:
    Filesystem* _fs;
};

class FileHandleReader : public FileHandle
{
public:
    FileHandleReader(Filesystem* fs, const string& filepath) :
        FileHandle(fs),
        _position(0)
    {
        ifstream fin(filepath);
        if (fin)
        {
            _isOpen = true;
            _data = std::move(vector<char>(std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>()));
        }
        fin.close();
    }

    bool seek(int32_t to, std::ios_base::seekdir way) override
    {
        switch (way)
        {
        case std::ios_base::cur:
            _position += to;
        break;
        case std::ios_base::beg:
            _position = to;
        break;
        case std::ios_base::end:
        default:
            _position = static_cast<int32_t>(_data.size()) - to;
        break;
        }

        return true;
    }

    int32_t tell() override
    {
        return _position;
    }
    
    vector<char> read(streamsize size) override
    {
        int32_t offset = std::max(0, _position);
        int32_t bytes = std::min(static_cast<int32_t>(size), static_cast<int32_t>(_data.size()) - offset);

        if (bytes <= 0)
            return {};

        vector<char> buffer(_data.begin() + offset, _data.begin() + offset + bytes);
        bytes -= buffer.size();

        while (bytes--)
        {
            buffer.push_back(0);
        }

        _position += buffer.size();
        return buffer;
    }

    bool eof() const override
    {
        return _position >= static_cast<int32_t>(_data.size());
    }

protected:
    void _close() override
    {
        _position = 0;
        _data.clear();
    }

    bool _isReader() const override
    {
        return true;
    }
private:
    int32_t _position;
    vector<char> _data;
};

class FileHandleWriter : public FileHandle
{
public:
    FileHandleWriter(Filesystem* fs, const string& filepath) :
        FileHandle(fs)
    {
        _stream.open(filepath);
        _isOpen = _stream.is_open();
    }

    void write(const vector<char>& data) override
    {
        _stream.write(data.data(), data.size());
    }

    bool seek(int32_t to, std::ios_base::seekdir way) override
    {
        // we may have read past the eof
        _stream.clear();

        _stream.seekg(to, way);

        if (_stream.fail() || _stream.bad())
        {
            _stream.clear();
            return false;
        }

        return true;
    }

    int32_t tell() override
    {
        return static_cast<int32_t>(_stream.tellg());
    }
protected:
    void _close() override
    {
        _stream.close();
    }

    bool _isReader() const override
    {
        return false;
    }

private:
    fstream _stream;
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

    FileHandle* pfh = create(lua, filepath, mode);
    if (!pfh)
    {
        return ValuePack::ret(lua, Value::nil, filepath);
    }
    // if create succeeded, the the top of the stack is the user data;
    return 1;
}

int Filesystem::read(lua_State* lua)
{
    FileHandle* pfh = getFileHandle(lua);
    if (!pfh->canRead())
    {
        return ValuePack::ret(lua, Value::nil, "bad file descriptor");
    }

    static constexpr double max_size = std::numeric_limits<int32_t>::max();
    static constexpr double min_size = std::numeric_limits<int32_t>::min();

    double dsize = Value::check(lua, 2, "number").toNumber();
    dsize = std::max(min_size, std::min(max_size, dsize));

    vector<char> data = pfh->read(static_cast<int32_t>(dsize));

    if (data.empty() && (dsize > 0 || pfh->eof()))
    {
        return ValuePack::ret(lua, Value::nil);
    }

    return ValuePack::ret(lua, data);
}

int Filesystem::write(lua_State* lua)
{
    FileHandle* pfh = getFileHandle(lua);
    if (!pfh->canWrite())
    {
        return ValuePack::ret(lua, Value::nil, "bad file descriptor");
    }

    vector<char> data = Value::check(lua, 2, "string").toRawString();
    pfh->write(data);

    return ValuePack::ret(lua, true);
}

int Filesystem::close(lua_State* lua)
{
    FileHandle* pfh = getFileHandle(lua);
    release(pfh);
    if (!pfh->isOpen())
    {
        return ValuePack::ret(lua, Value::nil, "bad file descriptor");
    }
    
    pfh->close();
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
    FileHandle* pfh = getFileHandle(lua);
    if (!pfh->isOpen())
    {
        return ValuePack::ret(lua, Value::nil, "bad file descriptor");
    }

    string whence = Value::check(lua, 2, "string").toString(); 
    streamoff to = (streamoff)Value::check(lua, 3, "number", "nil").Or(0).toNumber();

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

    if (!pfh->seek(to, way))
    {
        return ValuePack::ret(lua, Value::nil, "invalid offset");
    }

    return ValuePack::ret(lua, static_cast<double>(pfh->tell()));
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

FileHandle* Filesystem::getFileHandle(lua_State* lua) const
{
    const Value& handle = Value::check(lua, 1, "userdata");
    return reinterpret_cast<FileHandle*>(handle.toPointer());
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

FileHandle* Filesystem::create(lua_State* lua, const string& filepath, fstream::openmode mode)
{
    string fullpath = path() + clean(filepath, true, false);
    FileHandle* pfh = nullptr;
    
    if (mode & fstream::in)
    {
        void* pAlloc = lua_newuserdata(lua, sizeof(FileHandleReader));
        pfh = new(pAlloc) FileHandleReader(this, fullpath);
    }
    else
    {
        void* pAlloc = lua_newuserdata(lua, sizeof(FileHandleWriter));
        pfh = new(pAlloc) FileHandleWriter(this, fullpath);
    }

    if (!pfh->isOpen())
    {
        pfh->close(); // release resources if any
        lua_pop(lua, 1); // releaes userdata
        return nullptr;
    }

    _handles.insert(pfh);
    return pfh;
}

void Filesystem::release(FileHandle* pfh)
{
    _handles.erase(pfh);
}

