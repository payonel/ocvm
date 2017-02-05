#pragma once
#include <string>
#include <vector>
#include <map>
#include "value.h"

class Component;
typedef ValuePack (Component::*ComponentMethod)(const ValuePack& args);

class Component
{
public:
    Component(const std::string& type, const Value& init);
    virtual ~Component() {}
    std::string type() const;
    ValuePack invoke(const std::string& methodName, const ValuePack& args);

    static std::string make_address();

    template<typename... Ts>
    void invoke(const std::string& methodName, Ts... args)
    {
        ValuePack vec = Value::pack(args...);
        invoke(methodName, vec);
    }
protected:
    void add(const std::string& methodName, ComponentMethod method);

    template <typename Derived>
    void add(const std::string& methodName, ValuePack (Derived::*derivedMethod)(const ValuePack&))
    {
        add(methodName, static_cast<ComponentMethod>(derivedMethod));
    }
private:
    std::map<std::string, ComponentMethod> _methods;
    std::string _type;
    std::string _address;
};
