#pragma once
#include "luaproxy.h"
#include <string>
using std::string;

class UnicodeApi : public LuaProxy
{
public:
    static UnicodeApi* get();

    static string wtrunc(const string& text, int width);
    static bool isWide(const string& text);
    static string upper(const string& text);
    static string tochar(int n);
    static int wlen(const string& text);
    static int len(const string& text);
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
private:
    UnicodeApi();
};
