#pragma once
#include "model/luaproxy.h"
#include <unordered_map>
#include <vector>
using std::unordered_map;
using std::vector;

class UnicodeIterator
{
public:
  struct UnicodeIt
  {
    bool operator!=(const UnicodeIt& other) const;
    void operator++();
    vector<char> operator*() const;
    size_t next() const;

    UnicodeIterator& parent;
    size_t start;
  };

  UnicodeIt begin();
  UnicodeIt end();
  const char* source;
  const size_t size;
};

class UnicodeApi : public LuaProxy
{
public:
  static UnicodeApi* get();
  static UnicodeIterator subs(const vector<char>& src);

  static vector<char> wtrunc(const vector<char>& text, const size_t width);
  static bool isWide(const vector<char>& text);
  static vector<char> upper(const vector<char>& text);
  static vector<char> tochar(const uint32_t n);
  static uint32_t tocodepoint(const vector<char>& text, const size_t index = 0);
  static size_t wlen(const vector<char>& text);
  static size_t len(const vector<char>& text);
  static vector<char> sub(const vector<char>& text, int from, int to);
  static int charWidth(const vector<char>& text, bool bAlreadySingle = false);
  static vector<char> reverse(const vector<char>& text);
  static vector<char> lower(const vector<char>& text);

  int wtrunc(lua_State* lua);
  int isWide(lua_State* lua);
  int upper(lua_State* lua);
  int tochar(lua_State* lua);
  int wlen(lua_State* lua);
  int len(lua_State* lua);
  int sub(lua_State* lua);
  int charWidth(lua_State* lua);
  int reverse(lua_State* lua);
  int lower(lua_State* lua);

  static inline vector<char> toRawString(const string& text)
  {
    return vector<char>(text.begin(), text.end());
  }

  static inline string toString(const vector<char>& buffer)
  {
    return string(buffer.begin(), buffer.end());
  }

  static bool configure(const string& fonts_path);

private:
  UnicodeApi();
  static std::unordered_map<uint32_t, int> font_width;
};
