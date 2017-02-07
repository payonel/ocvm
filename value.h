#pragma once

#include <string>
#include <vector>
#include <map>
#include <memory>

class Value;
typedef std::vector<Value> ValuePack;
typedef std::map<Value, Value>::value_type ValuePair;

class lua_State;

class Value
{
public:
    Value(const std::string& v);
    Value(bool b);
    Value(double d);
    Value(const void* p);
    Value(int n) : Value(double(n)) {}
    Value(const char* cstr) : Value(std::string(cstr)) {}
    Value();

    static Value make(lua_State* lua, int index = -1);
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
    void* toPointer() const;
    const Value& metatable() const;

    static const Value& select(const ValuePack& pack, size_t index);

    // table functions
    const Value& get(const Value& key) const;
    Value& get(const Value& key);
    void set(const Value& key, const Value& value);
    std::vector<ValuePair> pairs() const;

    std::string serialize() const;
    operator bool() const;
protected:
    static void getmetatable(Value& v, lua_State* lua, int index);
private:
    std::string _type;
    std::string _string;
    bool _bool = false;
    double _number = 0;
    void* _pointer = nullptr;
    std::shared_ptr<Value> _pmetatable;
    std::map<Value, Value> _table;
};

bool operator< (const Value& a, const Value& b);
