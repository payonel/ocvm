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

ValuePack UnicodeApi::sub(const ValuePack& args)
{
    string text = Value::check(args, 0, "string").toString();
    size_t len = text.size();
    size_t from = (size_t)Value::check(args, 1, "number").toNumber();
    size_t to = (size_t)Value::check(args, 2, "number", "nil").Or(len).toNumber();

    // respect 1-based lua
    if (from < 0)
        from = len + from + 1;
    if (to < 0)
        to = len + to + 1;

    if (from > to || to == 0 || from > len)
    {
        return ValuePack { "" };
    }

    to = std::min(to, len);

    // indexes are in lua form, now convert
    from--;
    to--;

    return ValuePack { text.substr(from, to - from + 1) };
}

ValuePack UnicodeApi::len(const ValuePack& args)
{
    return ValuePack { Value::check(args, 0, "string").toString().size() };
}

ValuePack UnicodeApi::wlen(const ValuePack& args)
{
    return ValuePack { Value::check(args, 0, "string").toString().size() };
}

