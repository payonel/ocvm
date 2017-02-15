#include "unicode.h"

UnicodeApi::UnicodeApi() : LuaProxy("unicode")
{
    add("sub", &UnicodeApi::sub);
    add("len", &UnicodeApi::len);
    add("wlen", &UnicodeApi::wlen);
}

UnicodeApi* UnicodeApi::get()
{
    static UnicodeApi it;
    return &it;
}

string UnicodeApi::sub(const string& text, size_t from, size_t to)
{
    size_t len = text.size();

    // respect 1-based lua
    if (from < 0)
        from = len + from + 1;
    if (to < 0)
        to = len + to + 1;

    if (from > to || to == 0 || from > len)
    {
        return "";
    }

    to = std::min(to, len);

    // indexes are in lua form, now convert
    from--;
    to--;

    return text.substr(from, to - from + 1);
}

size_t UnicodeApi::len(const string& text)
{
    return text.size();
}

size_t UnicodeApi::wlen(const string& text)
{
    return text.size();
}

int UnicodeApi::sub(lua_State* lua)
{
    string text = Value::check(lua, 0, "string").toString();
    size_t len = text.size();
    size_t from = (size_t)Value::check(lua, 1, "number").toNumber();
    size_t to = (size_t)Value::check(lua, 2, "number", "nil").Or(len).toNumber();

    Value(sub(text, from, to)).push(lua);
    return 1;
}

int UnicodeApi::len(lua_State* lua)
{
    Value(len(Value::check(lua, 0, "string").toString())).push(lua);
    return 1;
}

int UnicodeApi::wlen(lua_State* lua)
{
    Value(wlen(Value::check(lua, 0, "string").toString())).push(lua);
    return 1;
}

