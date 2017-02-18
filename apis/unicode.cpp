#include "unicode.h"

UnicodeApi::UnicodeApi() : LuaProxy("unicode")
{
    cadd("wtrunc", &UnicodeApi::wtrunc);
    cadd("isWide", &UnicodeApi::isWide);
    cadd("upper", &UnicodeApi::upper);
    cadd("char", &UnicodeApi::tochar);
    cadd("wlen", &UnicodeApi::wlen);
    cadd("len", &UnicodeApi::len);
    cadd("sub", &UnicodeApi::sub);
    cadd("charWidth", &UnicodeApi::charWidth);
    cadd("reverse", &UnicodeApi::reverse);
    cadd("lower", &UnicodeApi::lower);
}

UnicodeApi* UnicodeApi::get()
{
    static UnicodeApi it;
    return &it;
}

string UnicodeApi::wtrunc(const string& text, int width)
{
    string result;
    int length = len(text);
    for (int i = 0; i < length; i++)
    {
        string next = result + sub(text, i, i);
        if (wlen(next) >= width)
            break;
        result = next;
    }
    return result;
}

bool UnicodeApi::isWide(const string& text)
{
    return wlen(text) > 1;
}

string UnicodeApi::upper(const string& text)
{
    string r;
    for (const auto& c : text)
        r += ::toupper(c);
    return r;
}

string UnicodeApi::tochar(int n)
{
    return string{char(n)};
}

int UnicodeApi::wlen(const string& text)
{
    return text.size();
}

int UnicodeApi::len(const string& text)
{
    return text.size();
}

string UnicodeApi::sub(const string& text, int from, int to)
{
    int len = text.size();

    // respect 1-based lua
    if (from < 0)
        from = std::max(1, len + from + 1);
    if (to < 0)
        to = std::max(1, len + to + 1);

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

int UnicodeApi::charWidth(const string& text)
{
    return wlen(text);
}

string UnicodeApi::reverse(const string& text)
{
    string r;
    for (int i = text.size(); i > 0; i--)
    {
        r += text.at(i - 1);
    }
    return r;
}

string UnicodeApi::lower(const string& text)
{
    string r;
    for (const auto& c : text)
        r += ::tolower(c);
    return r;
}

int UnicodeApi::wtrunc(lua_State* lua)
{
    string text = Value::check(lua, 1, "string").toString();
    int width = Value::check(lua, 2, "number").toNumber();
    if (wlen(text) < width)
        luaL_error(lua, "index out of range");
    return ValuePack::push(lua, wtrunc(text, width));
}

int UnicodeApi::isWide(lua_State* lua)
{
    return ValuePack::push(lua, isWide(Value::check(lua, 1, "string").toString()));
}

int UnicodeApi::upper(lua_State* lua)
{
    return ValuePack::push(lua, upper(Value::check(lua, 1, "string").toString()));
}

int UnicodeApi::tochar(lua_State* lua)
{
    return ValuePack::push(lua, tochar(Value::check(lua, 1, "number").toNumber()));
}

int UnicodeApi::wlen(lua_State* lua)
{
    return ValuePack::push(lua, wlen(Value::check(lua, 1, "string").toString()));
}

int UnicodeApi::len(lua_State* lua)
{
    return ValuePack::push(lua, len(Value::check(lua, 1, "string").toString()));
}

int UnicodeApi::sub(lua_State* lua)
{
    string text = Value::check(lua, 1, "string").toString();
    int len = text.size();
    int from = Value::check(lua, 2, "number").toNumber();
    int to = Value::check(lua, 3, "number", "nil").Or(len).toNumber();

    return ValuePack::push(lua, sub(text, from, to));
}

int UnicodeApi::charWidth(lua_State* lua)
{
    return wlen(lua);
}

int UnicodeApi::reverse(lua_State* lua)
{
    return ValuePack::push(lua, reverse(Value::check(lua, 1, "string").toString()));
}

int UnicodeApi::lower(lua_State* lua)
{
    return ValuePack::push(lua, lower(Value::check(lua, 1, "string").toString()));
}

