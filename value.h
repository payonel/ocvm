#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

using std::string;
using std::vector;
using std::map;
using std::shared_ptr;
using std::ostream;

class lua_State;
class Value
{
public:
    Value(const string& v);
    Value(bool b);
    Value(double d);
    Value(const void* p, bool bLight);
    Value(int n) : Value(double(n)) {}
    Value(const char* cstr) : Value(string(cstr)) {}
    Value(int64_t n) : Value(double(n)) {}
    Value(size_t n) : Value(double(n)) {}
    Value(lua_State*);
    Value(lua_State*, int);
    Value();

    static string stack(lua_State* state);
    static Value table();
    static const Value nil;

    const Value& Or(const Value& def) const;

    void push(lua_State* lua) const;

    void insert(const Value& value);
    int len() const;
    string type() const;
    int type_id() const;
    string toString() const;
    double toNumber() const;
    bool toBool() const;
    void* toPointer() const;
    lua_State* toThread() const;
    int status() const;

    // static const Value& select(const ValuePack& pack, size_t index);
    static Value check(lua_State* lua, size_t index, const string& required, const string& optional = "");

    // table functions
    vector<string> keys() const;
    const Value& get(const string& key) const;
    Value& get(const string& key);
    const Value& get(int index) const;
    Value& get(int index);
    void set(const string& key, const Value& value);
    void set(int key, const Value& value);
    bool contains(int key) const;
    bool contains(const string& key) const;

    string serialize(int spacey = 0) const;
    explicit operator bool() const;
    bool operator< (const Value& rhs) const;
private:
    string _type;
    int _id;
    string _string;
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

    inline static int push(lua_State* lua, const char* arg)
    {
        Value(arg).push(lua);
        return 1;
    }

    inline static int push(lua_State* lua, const Value& v)
    {
        v.push(lua);
        return 1;
    }

    template <typename T>
    inline static int push(lua_State* lua, const T& arg)
    {
        Value(arg).push(lua);
        return 1;
    }

    template <typename T, typename ...Ts>
    inline static int push(lua_State* lua, const T& arg, const Ts&... args)
    {
        return push(lua, arg) + push(lua, args...);
    }
};
ostream& operator << (ostream& os, const ValuePack& pack);
