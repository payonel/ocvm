#include "unicode.h"

static const uint32_t end_1_byte = 0x00000080;
static const uint32_t end_2_byte = 0x00000800;
static const uint32_t end_3_byte = 0x00010000;
static const uint32_t end_4_byte = 0x00200000;
static const uint32_t end_5_byte = 0x04000000;
static const uint32_t end_6_byte = 0x80000000;

static const unsigned char continuation_bit = 0x80; // 10xx xxxx
static const unsigned char set_2_bytes_bits = 0xC0; // 110x xxxx
static const unsigned char set_3_bytes_bits = 0xE0; // 1110 xxxx
static const unsigned char set_4_bytes_bits = 0xF0; // 1111 0xxx
static const unsigned char set_5_bytes_bits = 0xF8; // 1111 10xx
static const unsigned char set_6_bytes_bits = 0xFC; // 1111 110x

static const unsigned char continuation_mask = 0x3F; // 0011 1111
static const unsigned char set_2_mask        = 0x1F; // 0001 1111
static const unsigned char set_3_mask        = 0x0F; // 0000 1111
static const unsigned char set_4_mask        = 0x07; // 0000 0111
static const unsigned char set_5_mask        = 0x03; // 0000 0011
static const unsigned char set_6_mask        = 0x01; // 0000 0001

static inline bool tryGetNextIndex(const string& text, const size_t& start_index, size_t* pNext)
{
    if (start_index >= text.size())
        return false;

    unsigned char c = text.at(start_index);
    size_t step = 0;
    if (c < end_1_byte) step = 1;
    else if (c < end_2_byte) step = 2;
    else if (c < end_3_byte) step = 3;
    else if (c < end_4_byte) step = 4;
    else if (c < end_5_byte) step = 5;
    else if (c < end_6_byte) step = 6;
    else return false; // unsupported char range

    *pNext = start_index + step;
    return true;
}

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

string UnicodeApi::wtrunc(const string& text, const size_t width)
{
    size_t current_width = 0;
    size_t start = 0;
    size_t next;
    while (tryGetNextIndex(text, start, &next))
    {
        current_width += charWidth(text.substr(start, next - start));
        if (current_width >= width)
        {
            text.substr(0, start);
            break;
        }
        start = next;
    }

    return text;
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

string UnicodeApi::tochar(const uint32_t codepoint)
{
    char buffer[6] {};

    if (codepoint < end_1_byte)
    {
        buffer[0] = codepoint;
    }
    else if (codepoint < end_2_byte)
    {
        buffer[0] = set_2_bytes_bits | (set_2_mask & codepoint >> 6);
        buffer[1] = continuation_bit | (continuation_mask & codepoint);
    }
    else if (codepoint < end_3_byte)
    {
        buffer[0] = set_3_bytes_bits | (set_3_mask & codepoint >> 12);
        buffer[1] = continuation_bit | (continuation_mask & codepoint >> 6);
        buffer[2] = continuation_bit | (continuation_mask & codepoint);
    }
    else if (codepoint < end_4_byte)
    {
        buffer[0] = set_4_bytes_bits | (set_4_mask & codepoint >> 18);
        buffer[1] = continuation_bit | (continuation_mask & codepoint >> 12);
        buffer[2] = continuation_bit | (continuation_mask & codepoint >> 6);
        buffer[3] = continuation_bit | (continuation_mask & codepoint);
    }
    else if (codepoint < end_5_byte)
    {
        buffer[0] = set_5_bytes_bits | (set_5_mask & codepoint >> 24);
        buffer[1] = continuation_bit | (continuation_mask & codepoint >> 18);
        buffer[2] = continuation_bit | (continuation_mask & codepoint >> 12);
        buffer[3] = continuation_bit | (continuation_mask & codepoint >> 6);
        buffer[4] = continuation_bit | (continuation_mask & codepoint);
    }
    else if (codepoint < end_6_byte)
    {
        buffer[0] = set_6_bytes_bits | (set_6_mask & codepoint >> 30);
        buffer[1] = continuation_bit | (continuation_mask & codepoint >> 24);
        buffer[2] = continuation_bit | (continuation_mask & codepoint >> 18);
        buffer[3] = continuation_bit | (continuation_mask & codepoint >> 12);
        buffer[4] = continuation_bit | (continuation_mask & codepoint >> 6);
        buffer[5] = continuation_bit | (continuation_mask & codepoint);
    }
    // else invalid

    return string{buffer};
}

size_t UnicodeApi::wlen(const string& text)
{
    size_t result = 0;
    size_t start = 0;
    size_t next;
    while (tryGetNextIndex(text, start, &next))
    {
        result += charWidth(text.substr(start, next - start));
        start = next;
    }

    return result;
}

size_t UnicodeApi::len(const string& text, size_t index)
{
    size_t result = 0;
    size_t last = 0;
    size_t next;
    while (tryGetNextIndex(text, last, &next))
    {
        last = next;
        result++;
    }

    return result;
}

string UnicodeApi::sub(const string& text, int from, int to)
{
    size_t unicode_len = UnicodeApi::len(text);

    // respect 1-based lua
    if (from < 0)
        from = std::max(1, (int)unicode_len + from + 1);
    if (to < 0)
        to = std::max(1, (int)unicode_len + to + 1);

    if (from > to || to == 0 || from > (int)unicode_len)
    {
        return "";
    }

    to = std::min(to, (int)unicode_len);

    // indexes are in lua form, now convert
    from--;
    size_t requested_length = to - from;

    size_t start_index;

    size_t real_index = 0;
    size_t next = 0;
    string result = "";

    for (; from > 0 && real_index < unicode_len; )
    {
        if (!tryGetNextIndex(text, real_index, &next))
            return "";
        real_index = next;
        from--;
    }
    start_index = real_index;
    for (; requested_length > 0; requested_length--)
    {
        real_index = next;
        if (!tryGetNextIndex(text, real_index, &next))
            return "";
    }

    return text.substr(start_index, real_index - start_index + 1);
}

int UnicodeApi::charWidth(const string& text)
{
    string single = UnicodeApi::sub(text, 1, 1);
    size_t size = single.size();
    if (size == 0) return 0;
    if (size == 1) return 1;
    return 2;
}

string UnicodeApi::reverse(const string& text)
{
    string r;
    size_t len = UnicodeApi::len(text);
    for (size_t i = 0; i < len; i++)
    {
        r = UnicodeApi::sub(text, i + 1, i + 1) + r;
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
    size_t width = Value::check(lua, 2, "number").toNumber();
    if (wlen(text) < width)
        luaL_error(lua, "index out of range");
    return ValuePack::ret(lua, wtrunc(text, width));
}

int UnicodeApi::isWide(lua_State* lua)
{
    return ValuePack::ret(lua, isWide(Value::check(lua, 1, "string").toString()));
}

int UnicodeApi::upper(lua_State* lua)
{
    return ValuePack::ret(lua, upper(Value::check(lua, 1, "string").toString()));
}

int UnicodeApi::tochar(lua_State* lua)
{
    return ValuePack::ret(lua, tochar(Value::check(lua, 1, "number").toNumber()));
}

int UnicodeApi::wlen(lua_State* lua)
{
    return ValuePack::ret(lua, wlen(Value::check(lua, 1, "string").toString()));
}

int UnicodeApi::len(lua_State* lua)
{
    return ValuePack::ret(lua, len(Value::check(lua, 1, "string").toString()));
}

int UnicodeApi::sub(lua_State* lua)
{
    string text = Value::check(lua, 1, "string").toString();
    int len = text.size();
    int from = Value::check(lua, 2, "number").toNumber();
    int to = Value::check(lua, 3, "number", "nil").Or(len).toNumber();

    return ValuePack::ret(lua, sub(text, from, to));
}

int UnicodeApi::charWidth(lua_State* lua)
{
    return wlen(lua);
}

int UnicodeApi::reverse(lua_State* lua)
{
    return ValuePack::ret(lua, reverse(Value::check(lua, 1, "string").toString()));
}

int UnicodeApi::lower(lua_State* lua)
{
    return ValuePack::ret(lua, lower(Value::check(lua, 1, "string").toString()));
}

