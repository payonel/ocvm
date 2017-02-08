#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

class lua_State;
class Value;
struct ValuePack : public std::vector<Value>
{
    lua_State* state;
};

typedef std::map<Value, Value>::value_type ValuePair;

class Value
{
public:
    Value(const std::string& v);
    Value(bool b);
    Value(double d);
    Value(const void* p, bool bLight);
    Value(int n) : Value(double(n)) {}
    Value(const char* cstr) : Value(std::string(cstr)) {}
    Value(lua_State*);
    Value(lua_State*, int);
    Value();

    static Value table();
    static Value nil;

    static ValuePack pack()
    {
        return ValuePack();
    }

    template<typename T, typename... Ts>
    static ValuePack pack(T arg, Ts... args)
    {
        ValuePack vec = pack(args...);
        vec.insert(vec.begin(), Value(arg));
        return vec;
    }

    void push(lua_State* lua) const;

    std::string type() const;
    std::string toString() const;
    double toNumber() const;
    bool toBool() const;
    void* toPointer() const;
    const Value& metatable() const;

    static const Value& select(const ValuePack& pack, size_t index);
    static const Value& check(const ValuePack& pack, size_t index, const std::string& required, const std::string& optional = "");

    // table functions
    const Value& get(const Value& key) const;
    Value& get(const Value& key);
    void set(const Value& key, const Value& value);
    std::vector<ValuePair> pairs() const;

    std::string serialize() const;
    operator bool() const;
protected:
    void getmetatable(lua_State* lua, int index);
private:
    std::string _type;
    int _id;
    std::string _string;
    bool _bool = false;
    double _number = 0;
    void* _pointer = nullptr;
    std::shared_ptr<Value> _pmetatable;
    std::map<Value, Value> _table;
};

bool operator< (const Value& a, const Value& b);
