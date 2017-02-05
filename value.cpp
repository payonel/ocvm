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
    _string = v;
}

Value::Value()
{
    _type = "nil";
}

Value::Value(bool b)
{
    _type = "boolean";
    _bool = b;
}

Value::Value(double d)
{
    _type = "number";
    _number = d;
}

Value Value::table()
{
    Value t;
    t._type = "table";
    return t;
}


string Value::toString() const
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

    log << "failed to serialize Value[" << _type << "]\n";
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

Value Value::make(lua_State* lua, int index)
{
    int top = lua_gettop(lua);
    if (index <= top)
    {
        int type = lua_type(lua, index);
        string name = lua_typename(lua, type);
        switch (type)
        {
            case LUA_TSTRING:
                return Value(lua_tostring(lua, index));
            break;
            case LUA_TBOOLEAN:
                return Value(lua_toboolean(lua, index));
            break;
            case LUA_TNUMBER:
                return Value(lua_tonumber(lua, index));
            break;
            case LUA_TNIL:
                return Value::nil;
            break;
            case LUA_TTABLE:
                Value table = Value::table();
                lua_pushnil(lua); // push nil as first key for next()
                while (lua_next(lua, index))
                {
                    // key: -1
                    // value: -2
                    Value key = Value::make(lua, -2);
                    Value value = Value::make(lua, -1);
                    table.set(key, value);
                    lua_pop(lua, 1);
                }
                return table;
            break;
        }
    }
    return Value::nil;
}
