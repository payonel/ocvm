#pragma once

#include <string>
#include <vector>
#include <map>

class Value;
typedef std::vector<Value> ValuePack;
typedef std::map<Value, Value>::value_type ValuePair;

class lua_State;

class Value
{
public:
    Value(const std::string& v);
    Value(const char* cstr) : Value(std::string(cstr)) {}
    Value(bool b);
    Value(double d);
    Value(int n) : Value(double(n)) {}
    Value();

    static Value make(lua_State* lua, int index);
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

    ValuePack unpack() const; // for table arrays

    std::string type() const;
    std::string toString() const;
    double toNumber() const;
    bool toBool() const;

    // table functions
    const Value& get(const Value& key) const;
    Value& get(const Value& key);
    void set(const Value& key, const Value& value);
    std::vector<ValuePair> pairs() const;

    std::string serialize() const;
    operator bool() const;
private:
    std::string _type;
    std::string _string;
    bool _bool;
    double _number;
    std::map<Value, Value> _table;
};

bool operator< (const Value& a, const Value& b);
