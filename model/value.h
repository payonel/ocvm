#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <lua.hpp>

using std::string;
using std::vector;
using std::map;
using std::shared_ptr;
using std::ostream;
using std::size_t;

class Value
{
public:
    Value(const vector<char>& v);
    Value(const string& v) : Value(vector<char>(v.begin(), v.end())) {}
    Value(bool b);
    Value(double d);
    Value(const void* p, bool bLight);
    Value(int n) : Value(double(n)) {}
    Value(const char* cstr) : Value(string(cstr)) {}
    Value(int64_t n) : Value(double(n)) {}
    Value(unsigned int n) : Value(double(n)) {}
    Value(size_t n) : Value(double(n)) {}
    Value(lua_State*);
    Value(lua_State*, int);
    Value();

    static string stack(lua_State* state);
    static Value table();
    static const Value nil;

    const Value& Or(const Value& def) const;

    void push(lua_State* lua) const;

    Value& insert(const Value& value);
    int len() const;
    string type() const;
    int type_id() const;

    string toString() const;
    vector<char> toRawString() const;
    double toNumber() const;
    bool toBool() const;
    void* toPointer() const;
    lua_State* toThread() const;
    int status() const;

    template <typename T>
    static T checkArg(lua_State* lua, int index, const T* pDefault = nullptr);

    // table functions
    vector<string> keys() const;
    const Value& get(const string& key) const;
    Value& get(const string& key);
    const Value& get(int index) const;
    Value& get(int index);
    Value& set(const string& key, const Value& value);
    Value& set(int key, const Value& value);
    bool contains(int key) const;
    bool contains(const string& key) const;

    string serialize(bool pretty = false, int depth = 0) const;
    explicit operator bool() const;
    bool operator< (const Value& rhs) const;
private:
    string _type;
    int _id;
    vector<char> _string;
    bool _bool = false;
    double _number = 0;
    void* _pointer = nullptr;
    lua_State* _thread = nullptr;
    int _thread_status = 0;
    map<string, Value> _stable;
    map<int, Value> _ntable;
};

struct ValuePack : public vector<Value>
{
    ValuePack(std::initializer_list<Value>);
    ValuePack() = default;

    static ValuePack pack(lua_State* lua);

    int push(lua_State* lua) const;

    template <typename ...Ts>
    inline static int ret(lua_State* lua, const Ts&... args)
    {
        ::lua_settop(lua, 0);
        return ValuePack::push_ret(lua, args...);
    }
private:
    inline static int push_ret(lua_State* lua, bool arg)
    {
        lua_pushboolean(lua, arg);
        return 1;
    }

    inline static int push_ret(lua_State* lua, int32_t arg)
    {
        lua_pushnumber(lua, arg);
        return 1;
    }

    inline static int push_ret(lua_State* lua, uint32_t arg)
    {
        lua_pushnumber(lua, arg);
        return 1;
    }

    inline static int push_ret(lua_State* lua, int64_t arg)
    {
        lua_pushnumber(lua, arg);
        return 1;
    }

    inline static int push_ret(lua_State* lua, uint64_t arg)
    {
        lua_pushnumber(lua, arg);
        return 1;
    }

    inline static int push_ret(lua_State* lua, double arg)
    {
        lua_pushnumber(lua, arg);
        return 1;
    }

    inline static int push_ret(lua_State* lua, float arg)
    {
        lua_pushnumber(lua, arg);
        return 1;
    }

    inline static int push_ret(lua_State* lua, const string& arg)
    {
        lua_pushstring(lua, arg.c_str());
        return 1;
    }

    inline static int push_ret(lua_State* lua, const char* arg)
    {
        lua_pushstring(lua, arg);
        return 1;
    }

    inline static int push_ret(lua_State* lua, const void* arg)
    {
        lua_pushlightuserdata(lua, const_cast<void*>(arg));
        return 1;
    }

    inline static int push_ret(lua_State* lua, const vector<char>& arg)
    {
        lua_pushlstring(lua, arg.data(), arg.size());
        return 1;
    }

    inline static int push_ret(lua_State* lua, const Value& v)
    {
        v.push(lua);
        return 1;
    }

    template <typename T, typename ...Ts>
    inline static int push_ret(lua_State* lua, const T& arg, const Ts&... args)
    {
        return ValuePack::push_ret(lua, arg) + ValuePack::push_ret(lua, args...);
    }
};
ostream& operator << (ostream& os, const ValuePack& pack);
