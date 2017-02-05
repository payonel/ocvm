#include "component.h"
#include "log.h"

#include <iostream>
#include <string>
using std::string;
using std::endl;

Component::Component(const string& type, const Value& init) :
    _type(type)
{
    Value v;
    v = init.get(1);
    if (!v)
        v = Value(make_address());
    _address = v.toString();
}

string Component::type() const
{
    return _type;
}

void Component::add(const string& methodName, ComponentMethod method)
{
    _methods[methodName] = method;
}

ValuePack Component::invoke(const string& methodName, const ValuePack& args)
{
    if (_methods.find(methodName) == _methods.end())
    {
        lout << "component does not have any such method: " << methodName << endl;
        return ValuePack();
    }

    ComponentMethod pmethod = _methods.at(methodName);
    return ((*this).*pmethod)(args);
}

string Component::make_address()
{
    return "";
}
