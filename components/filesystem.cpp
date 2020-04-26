#include "filesystem.h"
#include "apis/system.h"
#include "apis/userdata.h"
#include "drivers/fs_utils.h"
#include "model/client.h"
#include "model/host.h"
#include "model/log.h"
#include <fstream>
#include <limits>
using Logging::lout;
using std::ifstream;
using std::numeric_limits;

bool Filesystem::s_registered = Host::registerComponentType<Filesystem>("filesystem");

constexpr inline int32_t truncate_double(const double& value)
{
  const double max_size = std::numeric_limits<int32_t>::max();
  const double min_size = std::numeric_limits<int32_t>::min();
  return static_cast<int32_t>(std::min(max_size, std::max(min_size, value)));
}

constexpr inline int32_t truncate(const int32_t& value, const int32_t& min_value, const int32_t& max_value)
{
  return std::min(max_value, std::max(min_value, value));
}

class FileHandle : public UserData
{
public:
  FileHandle(Filesystem* fs)
      : _fs(fs)
  {
  }

  const Filesystem* fs() const
  {
    return _fs;
  }

  void dispose() override
  {
    close();
    _fs->release(this);
  }

  virtual bool isOpen() const
  {
    if (_fs == nullptr)
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

  virtual vector<char> read(int32_t size)
  {
    return {};
  }
  virtual void write(const vector<char>& data)
  {
  }
  virtual bool eof() const
  {
    return false;
  }

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
  FileHandleReader(Filesystem* fs, const string& filepath, fstream::openmode mode)
      : FileHandle(fs)
      , _position(0)
  {
    ifstream fin(filepath, mode);
    if (fin)
    {
      _isOpen = true;
      _data = { std::istreambuf_iterator<char>(fin), std::istreambuf_iterator<char>() };
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
      _position = static_cast<int32_t>(_data.size()) + to;
      break;
    }

    return true;
  }

  int32_t tell() override
  {
    return _position;
  }

  vector<char> read(int32_t size) override
  {
    int32_t size_trunc = static_cast<int32_t>(_data.size());
    size = truncate(size, 0, size_trunc - _position);
    if (size <= 0)
      return {};

    int32_t offset = truncate(_position, 0, size_trunc);
    int32_t read_from_data = truncate(size, 0, size_trunc - offset);
    int32_t fill_bytes = size - read_from_data;

    vector<char> buffer(_data.begin() + offset, _data.begin() + offset + read_from_data);

    while (fill_bytes--)
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
  FileHandleWriter(Filesystem* fs, const string& filepath, fstream::openmode mode)
      : FileHandle(fs)
  {
    _stream.open(filepath, mode);
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

Filesystem::Filesystem()
    : _isReadOnly(true)
    , _tmpfs(false)
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
  add("lastModified", &Filesystem::lastModified);
  add("spaceUsed", &Filesystem::spaceUsed);
  add("spaceTotal", &Filesystem::spaceTotal);
  add("remove", &Filesystem::remove);
  add("makeDirectory", &Filesystem::makeDirectory);
  add("rename", &Filesystem::rename);
}

bool Filesystem::onInitialize()
{
  Value source_uri = config().get(ConfigIndex::SourceUri).Or(false);
  if (source_uri.type() == "string") // loot disk
  {
    _isReadOnly = true;
    _src = fs_utils::make_proc_path(source_uri.toString());
    _tmpfs = false;
    if (!fs_utils::exists(_src))
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
    if (!fs_utils::exists(path()))
    {
      fs_utils::mkdir(path());
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
    // error in expected relative truncation, could not find root path
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
  static const string default_mode = "r";

  string filepath = Value::checkArg<string>(lua, 1);
  string mode_text = Value::checkArg<string>(lua, 2, &default_mode);

  static map<char, fstream::openmode> mode_map{
    { 'r', fstream::in },
    { 'w', fstream::out },
    { 'a', fstream::app },
    { 'b', fstream::binary }
  };
  fstream::openmode mode;
  bool uninitialized = true;
  for (char c : mode_text)
  {
    const auto& it = mode_map.find(c);
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

  bool bRead = false;
  bool bWrite = false;

  if (!uninitialized)
  {
    bRead = (mode & mode_map['r']) == mode_map['r'];
    bWrite = (mode & mode_map['w']) == mode_map['w'];
    bWrite |= (mode & mode_map['a']) == mode_map['a'];
  }

  if (uninitialized || (bRead && bWrite))
  {
    lout << "bad file mode: " << mode_text << endl;
    return ValuePack::ret(lua, Value::nil, filepath);
  }
  else if (isReadOnly() && bWrite)
  {
    return ValuePack::ret(lua, Value::nil, filepath);
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

  int32_t size = truncate_double(Value::checkArg<double>(lua, 2));
  vector<char> data = pfh->read(size);

  if (data.empty() && (size > 0 || pfh->eof()))
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

  vector<char> data = Value::checkArg<vector<char>>(lua, 2);
  pfh->write(data);

  return 0;
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
  return ValuePack::ret(lua);
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

  string new_label = Value::checkArg<string>(lua, 1);
  int stack = getLabel(lua);
  update(ConfigIndex::Label, new_label);
  return stack;
}

int Filesystem::list(lua_State* lua)
{
  string request_path = path() + clean(Value::checkArg<string>(lua, 1), true, false);
  auto listing = fs_utils::list(request_path);

  Value t = Value::table();
  for (const auto& item : listing)
  {
    string relative_item = clean(relative(request_path, item), false, true);
    if (fs_utils::isDirectory(item))
    {
      relative_item += "/";
    }

    t.insert(relative_item);
  }

  return ValuePack::ret(lua, t);
}

static bool hack_broken_dotdot(const string& path)
{
  static const string dot_end = "/..";
  size_t index = path.find(dot_end);
  if (index < path.size())
  {
    return path.size() - index == dot_end.size();
  }
  return false;
}

int Filesystem::isDirectory(lua_State* lua)
{
  string given = Value::checkArg<string>(lua, 1);
  string relpath = clean(given, true, true);
  if (hack_broken_dotdot(relpath))
    return ValuePack::ret(lua, Value::nil, given);
  return ValuePack::ret(lua, fs_utils::isDirectory(path() + relpath));
}

int Filesystem::exists(lua_State* lua)
{
  string given = Value::checkArg<string>(lua, 1);
  string given_clean = clean(given, true, true);
  if (hack_broken_dotdot(given_clean))
    return ValuePack::ret(lua, Value::nil, given);
  string fullpath = path() + given_clean;
  return ValuePack::ret(lua, fs_utils::exists(fullpath));
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

  string whence = Value::checkArg<string>(lua, 2);

  static const double default_to = 0;
  double to = truncate_double(Value::checkArg<double>(lua, 3, &default_to));

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

  if (!pfh->seek(static_cast<int32_t>(to), way))
  {
    return ValuePack::ret(lua, Value::nil, "invalid offset");
  }

  return ValuePack::ret(lua, static_cast<double>(pfh->tell()));
}

int Filesystem::size(lua_State* lua)
{
  string filepath = Value::checkArg<string>(lua, 1);
  return ValuePack::ret(lua, fs_utils::size(path() + clean(filepath, true, false)));
}

int Filesystem::lastModified(lua_State* lua)
{
  string filepath = Value::checkArg<string>(lua, 1);
  return ValuePack::ret(lua, fs_utils::lastModified(path() + clean(filepath, true, false)));
}

FileHandle* Filesystem::getFileHandle(lua_State* lua) const
{
  void* handle = Value::checkArg<void*>(lua, 1);
  return reinterpret_cast<FileHandle*>(handle);
}

int Filesystem::spaceUsed(lua_State* lua)
{
  return ValuePack::ret(lua, fs_utils::size(path(), true));
}

int Filesystem::spaceTotal(lua_State* lua)
{
  return ValuePack::ret(lua, numeric_limits<double>::infinity());
}

int Filesystem::remove(lua_State* lua)
{
  string filepath = Value::checkArg<string>(lua, 1);
  filepath = path() + clean(filepath, true, false);
  if (isReadOnly())
  {
    return ValuePack::ret(lua, false);
  }
  else if (!fs_utils::exists(filepath))
  {
    return ValuePack::ret(lua, Value::nil, "no such file or directory");
  }
  return ValuePack::ret(lua, fs_utils::remove(filepath));
}

int Filesystem::makeDirectory(lua_State* lua)
{
  string dirpath = Value::checkArg<string>(lua, 1);
  dirpath = path() + clean(dirpath, true, false);
  if (isReadOnly() || fs_utils::exists(dirpath))
  {
    return ValuePack::ret(lua, false);
  }
  fs_utils::mkdir(dirpath);
  return ValuePack::ret(lua, true);
}

static bool hack_broken_rename(const string& from, const string& to)
{
  if (to.find(from) != 0)
    return false;

  string tail = to.substr(from.size());
  if (tail.size() < 2)
    return false;

  if (tail.find("/") != 0)
    return false;

  if (tail.find("/", 1) != string::npos)
    return false;

  // broken!
  // a -> a/d should fail, but oc just deletes a
  return fs_utils::remove(from);
}

int Filesystem::rename(lua_State* lua)
{
  string raw_from = Value::checkArg<string>(lua, 1);
  string raw_to = Value::checkArg<string>(lua, 2);

  string from = path() + clean(raw_from, true, true);
  string to = path() + clean(raw_to, true, true);
  if (isReadOnly())
  {
    return ValuePack::ret(lua, false);
  }
  else if (fs_utils::exists(to))
  {
    return ValuePack::ret(lua, false);
  }
  else if (!fs_utils::exists(from))
  {
    return ValuePack::ret(lua, Value::nil, raw_from);
  }
  else if (hack_broken_rename(from, to))
  {
    return ValuePack::ret(lua, true);
  }
  return ValuePack::ret(lua, fs_utils::rename(from, to));
}

FileHandle* Filesystem::create(lua_State* lua, const string& filepath, fstream::openmode mode)
{
  string fullpath = path() + clean(filepath, true, false);
  FileHandle* pfh = nullptr;

  if (fs_utils::isDirectory(fullpath))
  {
    return nullptr;
  }

  if ((mode & fstream::in) == fstream::in)
  {
    auto pAlloc = UserDataAllocator(lua)(sizeof(FileHandleReader));
    pfh = new (pAlloc) FileHandleReader(this, fullpath, mode);
  }
  else
  {
    auto pAlloc = UserDataAllocator(lua)(sizeof(FileHandleWriter));
    pfh = new (pAlloc) FileHandleWriter(this, fullpath, mode);
  }

  if (!pfh->isOpen())
  {
    pfh->close();    // release resources if any
    lua_pop(lua, 1); // release userdata
    return nullptr;
  }

  _handles.insert(pfh);
  return pfh;
}

void Filesystem::release(FileHandle* pfh)
{
  _handles.erase(pfh);
}
