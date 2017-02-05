#pragma once

#include <string>

class Value
{
public:
    Value(const std::string& v);
    Value(bool b);
    Value();

    std::string type() const;
    std::string toString() const;
    bool toBool() const;

    std::string serialize() const;
private:
    std::string _type;
    std::string _string;
    bool _bool;
    operator bool() const;
};

bool operator< (const Value& a, const Value& b);

