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
class Value;
struct ValuePack : public vector<Value>
{
    lua_State* state;
    ValuePack(std::initializer_list<Value>);
    ValuePack(lua_State* state);
    ValuePack() = default;
};
ostream& operator << (ostream& os, const ValuePack& pack);

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
    void set(const Value& key, const Value& value);
    const Value& get(const Value& key) const;
    Value& get(const Value& key);
    const map<Value, Value>& pairs() const;
    map<Value, Value>& pairs();

    string serialize(bool bSpacey = false) const;
    operator bool() const;
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
    map<Value, Value> _table;
};

