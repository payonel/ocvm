#include "unicode.h"
#include "utils.h"
#include "log.h"
#include <sstream>
#include <fstream>
#include <limits>
using namespace std;

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

//static
std::unordered_map<uint32_t, int> UnicodeApi::font_width;

UnicodeApi::UnicodeApi() : LuaProxy("unicode")
{
    add("wtrunc", &UnicodeApi::wtrunc);
    add("isWide", &UnicodeApi::isWide);
    add("upper", &UnicodeApi::upper);
    add("char", &UnicodeApi::tochar);
    add("wlen", &UnicodeApi::wlen);
    add("len", &UnicodeApi::len);
    add("sub", &UnicodeApi::sub);
    add("charWidth", &UnicodeApi::charWidth);
    add("reverse", &UnicodeApi::reverse);
    add("lower", &UnicodeApi::lower);
}

UnicodeApi* UnicodeApi::get()
{
    static UnicodeApi it;
    return &it;
}

void UnicodeApi::configure(const Value& settings)
{
    font_width.clear();
    string fonts_file = settings.get("fonts").Or("").toString();
    if (utils::read(fonts_file))
    {
        ifstream file(fonts_file);

        while (file)
        {
            uint32_t key;
            file >> std::hex >> key;
            char delimiter = file.get();

            string bmp;
            getline(file, bmp);

            if (!file || delimiter != ':' || bmp.size() % 32 != 0)
            {
                lout << "unicode font load finished\n";
                break;
            }

            font_width[key] = bmp.size() / 32;
        }

        file.close();
    }
}

string UnicodeApi::wtrunc(const string& text, const size_t width)
{
    size_t current_width = 0;
    string result;
    for (const auto& sub : subs(text))
    {
        current_width += charWidth(sub);
        if (current_width >= width)
        {
            break;
        }
        result += sub;
    }

    return result;
}

bool UnicodeApi::isWide(const string& text)
{
    return charWidth(text) > 1;
}

string UnicodeApi::upper(const string& text)
{
    string r;
    for (const auto& c : text)
        r += ::toupper(c);
    return r;
}

uint32_t UnicodeApi::tocodepoint(const string& text, const size_t index)
{
    size_t end = text.size();
    if (index >= end) return 0;
    size_t open = end - index;

    unsigned char flags = text.at(index);
    uint32_t codepoint = flags;

    flags &= (((flags >> 1) & 0x40) | 0xBF);
    flags &= (((flags >> 1) & 0x20) | 0xDF);
    flags &= (((flags >> 1) & 0x10) | 0xEF);
    flags &= (((flags >> 1) & 0x08) | 0xF7);
    flags &= (((flags >> 1) & 0x04) | 0xFB);
    flags &= (((flags >> 1) & 0x02) | 0xFD);
    flags &= (((flags >> 1) & 0x01) | 0xFE);

    switch (flags)
    {
        case 0xC0: // 2 bytes
            codepoint = (open < 2) ? 0 : 
                ((set_2_mask & codepoint) << 6) |
                (continuation_mask & text.at(index + 1));
            break;
        case 0xE0: // 3 bytes
            codepoint = (open < 3) ? 0 : 
                ((set_3_mask & codepoint) << 12) |
                ((continuation_mask & text.at(index + 1)) << 6) |
                (continuation_mask & text.at(index + 2));
            break;
        case 0xF0: // 4 bytes
            codepoint = (open < 4) ? 0 : 
                ((set_4_mask & codepoint) << 18) |
                ((continuation_mask & text.at(index + 1)) << 12) |
                ((continuation_mask & text.at(index + 2)) << 6) |
                (continuation_mask & text.at(index + 3));
            break;
        case 0xF8: // 5 bytes
            codepoint = (open < 5) ? 0 : 
                ((set_5_mask & codepoint) << 24) |
                ((continuation_mask & text.at(index + 1)) << 18) |
                ((continuation_mask & text.at(index + 2)) << 12) |
                ((continuation_mask & text.at(index + 3)) << 6) |
                (continuation_mask & text.at(index + 4));
            break;
        case 0xFC: // 6 bytes
            codepoint = (open < 6) ? 0 : 
                ((set_6_mask & codepoint) << 30) |
                ((continuation_mask & text.at(index + 1)) << 24) |
                ((continuation_mask & text.at(index + 2)) << 18) |
                ((continuation_mask & text.at(index + 3)) << 12) |
                ((continuation_mask & text.at(index + 4)) << 6) |
                (continuation_mask & text.at(index + 5));
            break;
        //default: // invalid, continuation bit, or ascii
    }

    return codepoint;
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
    size_t width = 0;
    for (const auto& sub : subs(text))
    {
        width += charWidth(sub);
    }

    return width;
}

size_t UnicodeApi::len(const string& text)
{
    size_t length = 0;
    auto iterator = subs(text);
    for (auto it = iterator.begin(); it != iterator.end(); ++it)
    {
        length++;
    }

    return length;
}

string UnicodeApi::sub(const string& text, int from, int to)
{
    if (from == 0)
        from = 1;
    if (to == 0)
        return "";

    size_t early_break = (from>0 && to>0) ? (size_t)std::max(from, to) : std::numeric_limits<size_t>::max();

    vector<string> parts;
    for (const auto& sub : subs(text))
    {
        parts.push_back(sub);
        if (parts.size() > early_break)
            break;
    }
    int numParts = parts.size();
    if (numParts == 0)
        return "";

    if (from < 0)
        from = std::max(1, from + numParts + 1);

    if (from > numParts)
        return "";

    if (to < 0)
        to += numParts + 1;
    to = std::min(numParts, to);
    
    // switch to zero-based index
    from--;
    string result;
    for (int i = from; i < to; i++)
    {
        result += parts.at(i);
    }

    return result;
}

int UnicodeApi::charWidth(const string& text)
{
    string single = UnicodeApi::sub(text, 1, 1);
    uint32_t codepoint = UnicodeApi::tocodepoint(single);
    const auto& it = font_width.find(codepoint);
    if (it == font_width.end())
        return 1;
    return it->second;
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

UnicodeIterator UnicodeApi::subs(const string& src)
{
    return UnicodeIterator{src};
}

bool UnicodeIterator::UnicodeIt::operator != (const UnicodeIterator::UnicodeIt& other) const
{
    return start != other.start;
}

void UnicodeIterator::UnicodeIt::operator++()
{
    start = next();
}

size_t UnicodeIterator::UnicodeIt::next() const
{
    const auto& src = parent.source;
    size_t length = src.size();
    if (start >= length)
        return length;

    unsigned char c = src.at(start);
    size_t step = 0;
    if (c <= end_1_byte) step = 1;
    else if (c <= set_2_bytes_bits) return length; // continuation bit, invalid point
    else if (c <= set_3_bytes_bits) step = 2;
    else if (c <= set_4_bytes_bits) step = 3;
    else if (c <= set_5_bytes_bits) step = 4;
    else if (c <= set_6_bytes_bits) step = 5;
    else return length; // unsupported char range

    return std::min(start + step, length);
}

string UnicodeIterator::UnicodeIt::operator*() const
{
    return parent.source.substr(start, next() - start);
}

UnicodeIterator::UnicodeIt UnicodeIterator::begin()
{
    return {*this, 0};
}

UnicodeIterator::UnicodeIt UnicodeIterator::end()
{
    return {*this, source.size()};
}

