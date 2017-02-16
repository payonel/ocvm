#include "value.h"
#include "log.h"

#include <limits>
#include <lua.hpp>

const Value Value::nil; // the nil

Value::Value(const string& v)
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

    switch (_id)
    {
        case LUA_TSTRING:
            _string = lua_tostring(lua, -1);
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
            _pointer = lua_touserdata(lua, -1);
        break;
        case LUA_TUSERDATA:
            _pointer = (void*)lua_topointer(lua, -1);
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
                    set(key._string, value);
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
        return _string;
    else
        return serialize();
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

void Value::set(const string& key, const Value& value)
{
    _stable[key] = value;
}
void Value::set(int key, const Value& value)
{
    _ntable[key] = value;
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

void Value::insert(const Value& value)
{
    int length = len();
    set(length + 1, value);
}

int Value::len() const
{
    if (type() == "string")
        return (int)_string.size();
    else if (type() == "table")
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

string Value::serialize(int spacey) const
{
    stringstream ss;
    string sp = spacey > 0 ? "\n" : "";
    string tab = spacey > 0 ? "\t" : "";
    if (_type == "string")
    {
        ss << "\"" + _string + "\"";
    }
    else if (_type == "boolean")
    {
        ss << (_bool ? "true" : "false");
    }
    else if (_type == "number")
    {
        bool neg = _number < 0;
        if ((neg ? -_number : _number) >= std::numeric_limits<double>::max())
            ss << (neg ? "-" : "") << "math.huge";
        else
            ss << _number;
    }
    else if (_type == "nil")
    {
        ss << "nil";
    }
    else if (_type == "table")
    {
        ss << "{" << sp;
        bool skipped_index = false;
        int count = len();
        for (int n = 1; n <= count; n++)
        {
            ss << tab;
            if (_ntable.find(n) == _ntable.end())
            {
                skipped_index = true;
                continue;
            }
            if (skipped_index)
                ss << "[" << n << "]=";
            ss << _ntable.at(n).serialize(spacey - 1);
            ss << "," << sp;
        }
        vector<string> ks = keys();
        for (const string& key : ks)
        {
            ss << tab << "[\"";
            ss << key;
            ss << "\"]=";
            ss << _stable.at(key).serialize(spacey - 1);
            ss << "," << sp;
        }
        ss << "}" << sp;
    }
    else
    {
        ss << "[" << _type << "]";
    }
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
            lua_pushstring(lua, _string.c_str());
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
