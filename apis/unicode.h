#pragma once
#include "luaproxy.h"
#include <string>
using std::string;

class UnicodeApi : public LuaProxy
{
public:
    static UnicodeApi* get();

    string sub(const string& text, size_t from, size_t to);
    size_t len(const string& text);
    size_t wlen(const string& text);

    ValuePack sub(lua_State* lua);
    ValuePack len(lua_State* lua);
    ValuePack wlen(lua_State* lua);
private:
    UnicodeApi();
};
