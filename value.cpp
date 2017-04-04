#include "value.h"
#include "log.h"

#include <limits>
#include <lua.hpp>

const Value Value::nil; // the nil

Value::Value(const vector<char>& v)
{
    _type = "string";
    _id = LUA_TSTRING;
    _string = v;
}

Value::Value()
{
    _id = LUA_TNIL;
    _type = "nil";
}

Value::Value(const void* p, bool bLight)
{
    _id = bLight ? LUA_TLIGHTUSERDATA : LUA_TUSERDATA;
    _type = "userdata";
    _pointer = const_cast<void*>(p);
}

Value::Value(bool b)
{
    _id = LUA_TBOOLEAN;
    _type = "boolean";
    _bool = b;
}

Value::Value(double d)
{
    _id = LUA_TNUMBER;
    _type = "number";
    _number = d;
}

Value::Value(lua_State* s)
{
    _type = "thread";
    _thread_status = lua_status(s);

    if (_thread_status == LUA_OK || _thread_status == LUA_YIELD)
    {
        int top = lua_gettop(s); // in case checking status adds to the stack
        for (int i = 1; i <= top; i++)
        {
            set(i, Value(s, i));
        }
        lua_settop(s, top);
    }

    _id = LUA_TTHREAD;
}

Value::Value(lua_State* lua, int index)
{
    lua_pushvalue(lua, index);
    _id = lua_type(lua, -1);
    _type = lua_typename(lua, _id);

    const char* p;
    size_t len;

    switch (_id)
    {
        case LUA_TSTRING:
            p = lua_tostring(lua, -1);
#if LUA_VERSION_NUM>502
            len = lua_strlen(lua, -1);
#else
            len = lua_rawlen(lua, -1);
#endif
            _string = vector<char>(p, p+len);
        break;
        case LUA_TBOOLEAN:
            _bool = lua_toboolean(lua, -1);
        break;
        case LUA_TNUMBER:
            _number = lua_tonumber(lua, -1);
        break;
        case LUA_TNIL:
        break;
        case LUA_TLIGHTUSERDATA:
            _pointer = const_cast<void*>(lua_topointer(lua, -1));
        break;
        case LUA_TUSERDATA:
            _pointer = lua_touserdata(lua, -1);
        break;
        case LUA_TTHREAD:
            _thread = lua_tothread(lua, -1);
        break;
        case LUA_TTABLE:
            lua_pushnil(lua); // push nil as first key for next()
            while (lua_next(lua, -2))
            {
                // return key, value
                Value value(lua, -1);
                Value key(lua, -2);
                if (key._id == LUA_TSTRING)
                {
                    set(key.toString(), value);
                }
                else if (key._id == LUA_TNUMBER)
                {
                    set((int)key._number, value);
                }
                else
                {
                    string msg = "value conversion for table keys does not support: ";
                    msg += key._type;
                    luaL_error(lua, msg.c_str());
                    return;
                }
                lua_pop(lua, 1); // only pop value, next retakes the key
            }
        break;
    }
    lua_pop(lua, 1);
}

Value Value::table()
{
    Value t;
    t._id = LUA_TTABLE;
    t._type = "table";
    return t;
}

string Value::toString() const
{
    if (type() == "string")
        return string(_string.begin(), _string.end());
    else
        return serialize();
}

vector<char> Value::toRawString() const
{
    return _string;
}

bool Value::toBool() const
{
    return _bool;
}

double Value::toNumber() const
{
    return _number;
}

void* Value::toPointer() const
{
    return _pointer;
}

lua_State* Value::toThread() const
{
    return _thread;
}

int Value::status() const
{
    return _thread_status;
}

const Value& Value::get(const string& key) const
{
    const auto& it = _stable.find(key);
    if (it == _stable.end())
        return Value::nil;
    
    return it->second;
}

Value& Value::get(const string& key)
{
    return _stable[key];
}

const Value& Value::get(int key) const
{
    const auto& it = _ntable.find(key);
    if (it == _ntable.end())
        return Value::nil;
    
    return it->second;
}

Value& Value::get(int key)
{
    return _ntable[key];
}

Value& Value::set(const string& key, const Value& value)
{
    _stable[key] = value;
    return *this;
}
Value& Value::set(int key, const Value& value)
{
    _ntable[key] = value;
    set("n", len());
    return *this;
}
bool Value::contains(int key) const
{
    return _ntable.find(key) != _ntable.end();
}
bool Value::contains(const string& key) const
{
    return _stable.find(key) != _stable.end();
}
vector<string> Value::keys() const
{
    vector<string> result;
    for (const auto& key : _stable)
        result.push_back(key.first);
    return result;
}

Value& Value::insert(const Value& value)
{
    int length = len();
    set(length + 1, value);
    return *this;
}

int Value::len() const
{
    if (_id == LUA_TSTRING)
        return (int)_string.size();
    else if (_id == LUA_TTHREAD || _id == LUA_TTABLE)
    {
        int max = 0;
        for (const auto& pair : _ntable)
        {
            max = std::max(max, pair.first);
        }
        return max;
    }

    return 0;
}

string Value::type() const
{
    return _type;
}

int Value::type_id() const
{
    return _id;
}

static string quote_string(const string& raw_string)
{
    if (raw_string.find("\"") == string::npos)
        return "\"" + raw_string + "\"";
    return "[=[" + raw_string + "]=]";
}

string Value::serialize(bool pretty, int depth) const
{
    stringstream ss;
    if (_id == LUA_TSTRING)
    {
        ss << quote_string(string(_string.begin(), _string.end()));
    }
    else if (_id == LUA_TBOOLEAN)
    {
        ss << (_bool ? "true" : "false");
    }
    else if (_id == LUA_TNUMBER)
    {
        bool neg = _number < 0;
        if ((neg ? -_number : _number) >= std::numeric_limits<double>::max())
            ss << (neg ? "-" : "") << "math.huge";
        else
            ss << _number;
    }
    else if (_id == LUA_TNIL)
    {
        ss << "nil";
    }
    else if (_id == LUA_TTABLE || _id == LUA_TTHREAD)
    {
        if (_id == LUA_TTHREAD)
        {
            ss << "[thread]";
        }

        vector<string> kvs;
        bool skipped_index = false;
        int count = len();
        for (int n = 1; n <= count; n++)
        {
            if (_ntable.find(n) == _ntable.end())
            {
                skipped_index = true;
                continue;
            }
            string key = "";
            if (skipped_index)
            {
                stringstream number_ss;
                number_ss << "[" << n << "]=";
                key = number_ss.str();
            }
            kvs.push_back(key + _ntable.at(n).serialize(pretty, depth + 1));
        }
        for (const string& key : keys())
        {
            string keytext = "[" + quote_string(key) + "]=";
            string value = _stable.at(key).serialize(pretty, depth + 1);
            kvs.push_back(keytext + value);
        }

        string newline = pretty ? "\n" : "";
        string indent = pretty ? "  " : "";
        string tab = "";
        for (int repeat = 0; pretty && repeat < depth; repeat++)
            tab += indent;

        ss << "{";
        bool any_linenew = false;

        for (const auto& entry : kvs)
        {
            bool linenew = entry.find_first_of("{}") != string::npos;
            any_linenew |= linenew;

            if (linenew)
                ss << newline << tab << indent;

            ss << entry << ",";
        }

        if (any_linenew)
            ss << newline << tab;

        ss << "}";
    }
    else
    {
        ss << "[" << _type << "]";
    }
    if (pretty && !depth)
        ss << "\n";

    return ss.str();
}

bool Value::operator< (const Value& rhs) const
{
    if (_id != rhs._id) return _id < rhs._id;
    switch (_id)
    {
        case LUA_TNUMBER:
            return _number < rhs._number;
        break;
        case LUA_TSTRING:
            return _string < rhs._string;
        break;
    }
    return false;
}

Value::operator bool() const
{
    return _type != "nil" && (_type != "boolean" || _bool);
}

void Value::push(lua_State* lua) const
{
    switch (_id)
    {
        case LUA_TSTRING:
            lua_pushlstring(lua, _string.data(), _string.size());
        break;
        case LUA_TBOOLEAN:
            lua_pushboolean(lua, _bool);
        break;
        case LUA_TNUMBER:
            lua_pushnumber(lua, _number);
        break;
        case LUA_TLIGHTUSERDATA:
            lua_pushlightuserdata(lua, _pointer);
        break;
        case LUA_TTHREAD:
            lua_pushthread(_thread);
        break;
        case LUA_TTABLE:
            lua_newtable(lua);
            for (const auto& pair : _ntable)
            {
                Value(pair.first).push(lua);
                pair.second.push(lua);
                lua_settable(lua, -3); // pop, pop
            }
            for (const auto& pair : _stable)
            {
                Value(pair.first).push(lua);
                pair.second.push(lua);
                lua_settable(lua, -3); // pop, pop
            }
        break;
        case LUA_TUSERDATA:
        case LUA_TNIL:
        default:
            lua_pushnil(lua);
        break;
    }
}

Value Value::check(lua_State* lua, size_t index, const string& required, const string& optional)
{
    int top = lua_gettop(lua);
    int id = LUA_TNIL;
    string type = "nil";
    if (index <= (size_t)top) // top:1, index:0 is max
    {
        id = lua_type(lua, index);
        type = lua_typename(lua, id);
    }

    if (type != required)
    {
        if (optional.empty() || type != optional)
        {
            luaL_error(lua, "bad arguments #%d (%s expected, got %s) ",
                index,
                required.c_str(),
                type.c_str());
        }
    }

    if (id == LUA_TNIL)
        return Value::nil;

    return Value(lua, index);
}

ValuePack::ValuePack(std::initializer_list<Value> values)
{
    for (const auto& v : values)
        push_back(v);
}

ostream& operator << (ostream& os, const ValuePack& pack)
{
    static const size_t length_limit = 120;
    stringstream ss;
    ss << "{";
    for (const auto& arg : pack)
    {
        ss << arg.serialize();
        ss << ",";
    }
    ss << "}";
    string text = ss.str();
    size_t cut = std::min(length_limit, text.find("\n"));
    os << text.substr(0, cut);
    return os;
}

const Value& Value::Or(const Value& def) const
{
    return (!*this) ? def : *this;
}

ValuePack ValuePack::pack(lua_State* lua)
{
    int top = lua_gettop(lua);
    ValuePack result;
    for (int i = 1; i <= top; i++)
    {
        result.push_back(Value(lua, i));
    }
    return result;
}

string Value::stack(lua_State* state)
{
    luaL_traceback(state, state, NULL, 1);
    string stacktrace = string(lua_tostring(state, -1));
    lua_pop(state, 1);
    return stacktrace;
}

int ValuePack::push(lua_State* lua) const
{
    lua_settop(lua, 0);
    for (const auto& v : *this)
    {
        v.push(lua);
    }
    return (int)size();
}
