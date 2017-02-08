#include "value.h"
#include "log.h"

#include <lua.hpp>

#include <sstream>
using std::string;
using std::map;
using std::vector;
using std::stringstream;

Value Value::nil; // the nil

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
    _type = "thread[";
    int before = lua_gettop(s); // in case checking status adds to the stack
    int status_id = lua_status(s);
    if (status_id == LUA_OK)
    {
        _type += "ok";
    }
    else if (status_id == LUA_YIELD)
    {
        _type += "yield";
    }
    else if (status_id == LUA_ERRRUN)
    {
        _type += "errrun";
    }
    else if (status_id == LUA_ERRSYNTAX)
    {
        _type += "syntax";
    }
    else if (status_id == LUA_ERRMEM)
    {
        _type += "memory";
    }
    else if (status_id == LUA_ERRGCMM)
    {
        _type += "gcmm";
    }
    else if (status_id == LUA_ERRERR)
    {
        _type += "errerr";
    }
    else
    {
        _type += "unknown";
    }
    _type += "]";
    _id = LUA_TTHREAD;
    lua_settop(s, before);
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

const Value& Value::metatable() const
{
    return _pmetatable ? *_pmetatable : Value::nil;
}

const Value& Value::select(const ValuePack& pack, size_t index)
{
    if (index >= pack.size())
    {
        return Value::nil;
    }

    return pack.at(index);
}

const Value& Value::get(const Value& key) const
{
    if (_table.find(key) == _table.end())
        return Value::nil;
    
    return _table.at(key);
}

Value& Value::get(const Value& key)
{
    if (_table.find(key) == _table.end())
        return Value::nil;
    
    return _table.at(key);
}

void Value::set(const Value& key, const Value& value)
{
    _table[key] = value;
}

vector<ValuePair> Value::pairs() const
{
    vector<ValuePair> vec;
    for (const auto& pair : _table)
    {
        vec.push_back(pair);
    }
    return vec;
}

string Value::type() const
{
    return _type;
}

string Value::serialize() const
{
    if (_type == "string")
    {
        return "\"" + _string + "\"";
    }
    else if (_type == "boolean")
    {
        return _bool ? "true" : "false";
    }
    else if (_type == "number")
    {
        stringstream ss;
        ss << _number;
        return ss.str();
    }
    else if (_type == "nil")
    {
        return "nil";
    }
    else if (_type == "table")
    {
        stringstream ss;
        ss << "{";
        for (const auto& pair : pairs())
        {
            ss << "[";
            ss << pair.first.serialize();
            ss << "] = ";
            ss << pair.second.serialize();
            ss << ",";
        }
        ss << "}";
        return ss.str();
    }

    lout << "failed to serialize Value[" << _type << "]\n";
    return "";
}

bool operator< (const Value& a, const Value& b)
{
    return a.serialize() < b.serialize();
}

Value::operator bool() const
{
    return _type != "nil" && (_type != "boolean" || _bool);
}

ValuePack Value::unpack() const
{
    return ValuePack();
}

void Value::getmetatable(Value& v, lua_State* lua, int index)
{
    if (v.type() == "table" || v.type() == "userdata")
    {
        if (lua_getmetatable(lua, index))
        {
            std::shared_ptr<Value> pmt(new Value);
            *pmt = Value::make(lua, -1);
            if (pmt->type() == "table")
            {
                v._pmetatable = pmt;
            }

            lua_pop(lua, 1);
        }
    }
}

Value Value::make(lua_State* lua, int index)
{
    int top = lua_gettop(lua);
    Value def;
    if (index <= top)
    {
        int type = lua_type(lua, index);
        string name = lua_typename(lua, type);
        def._type = name;
        switch (type)
        {
            case LUA_TSTRING:
                def = Value(lua_tostring(lua, index));
            break;
            case LUA_TBOOLEAN:
                def = Value((bool)lua_toboolean(lua, index));
            break;
            case LUA_TNUMBER:
                def = Value(lua_tonumber(lua, index));
            break;
            case LUA_TNIL:
                def = Value::nil;
            break;
            case LUA_TUSERDATA:
                def = Value(lua_touserdata(lua, index), false);
            break;
            case LUA_TLIGHTUSERDATA:
                def = Value((void*)lua_topointer(lua, index), true);
            break;
            case LUA_TTABLE:
                def = Value::table();
                index = index >= 0 ? index : (top + index + 1);
                lua_pushnil(lua); // push nil as first key for next()
                while (lua_next(lua, index))
                {
                    // return key, value
                    Value value = Value::make(lua, -1);
                    Value key = Value::make(lua, -2);
                    def.set(key, value);
                    lua_pop(lua, 1); // only pop value, next retakes the key
                }
            break;
        }
        Value::getmetatable(def, lua, index);
    }
    return def;
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
        case LUA_TTABLE:
            lua_newtable(lua);
            for (const auto& pair : _table)
            {
                pair.first.push(lua);
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

bool Value::checkArg(int index, const std::string& paramName, const std::string& expectedType)
{
    bool ok = type() == expectedType;
    return ok;
}

bool Value::checkArg(int index, const std::string& paramName, const std::string& expectedType, const std::string& optionalType)
{
    bool ok = type() == expectedType || type() == optionalType;
    return ok;
}
