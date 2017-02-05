#include "value.h"
#include "log.h"
using std::string;

Value::Value(const std::string& v)
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

string Value::toString() const
{
    return _string;
}

bool Value::toBool() const
{
    return _bool;
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
    else if (_type == "nil")
    {
        return "nil";
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
