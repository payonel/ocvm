#pragma once
#include "luaproxy.h"
#include <string>
#include <unordered_map>
using std::string;
using std::unordered_map;

class UnicodeIterator
{
public:
    struct UnicodeIt
    {
        bool operator != (const UnicodeIt& other) const;
        void operator++();
        string operator*() const;
        size_t next() const;

        UnicodeIterator& parent;
        size_t start;
    };

    UnicodeIt begin();
    UnicodeIt end();
    const string& source;
};

class UnicodeApi : public LuaProxy
{
public:
    static UnicodeApi* get();
    static UnicodeIterator subs(const string& src);

    static string wtrunc(const string& text, const size_t width);
    static bool isWide(const string& text);
    static string upper(const string& text);
    static string tochar(const uint32_t n);
    static uint32_t tocodepoint(const string& text, const size_t index = 0);
    static size_t wlen(const string& text);
    static size_t len(const string& text, size_t index = 0);
    static string sub(const string& text, int from, int to);
    static int charWidth(const string& text);
    static string reverse(const string& text);
    static string lower(const string& text);

    static int wtrunc(lua_State* lua);
    static int isWide(lua_State* lua);
    static int upper(lua_State* lua);
    static int tochar(lua_State* lua);
    static int wlen(lua_State* lua);
    static int len(lua_State* lua);
    static int sub(lua_State* lua);
    static int charWidth(lua_State* lua);
    static int reverse(lua_State* lua);
    static int lower(lua_State* lua);

    static void configure(const Value& settings);
private:
    UnicodeApi();
    static std::unordered_map<uint32_t, int> font_width;
};
