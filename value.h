#pragma once

#include <string>
#include <vector>

class Value;
typedef std::vector<Value> ValuePack;

class Value
{
public:
    Value(const std::string& v);
    Value(const char* cstr) : Value(std::string(cstr)) {}
    Value(bool b);
    Value(double d);
    Value(int n) : Value(double(n)) {}
    Value();

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

    std::string serialize() const;
private:
    std::string _type;
    std::string _string;
    bool _bool;
    double _number;
    operator bool() const;
};

bool operator< (const Value& a, const Value& b);
