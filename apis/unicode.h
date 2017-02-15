#pragma once
#include "luaproxy.h"
#include <string>
using std::string;

class UnicodeApi : public LuaProxy
{
public:
    static UnicodeApi* get();

    static string sub(const string& text, size_t from, size_t to);
    static size_t len(const string& text);
    static size_t wlen(const string& text);

    static int sub(lua_State* lua);
    static int len(lua_State* lua);
    static int wlen(lua_State* lua);
private:
    UnicodeApi();
};
