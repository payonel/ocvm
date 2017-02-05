#include "component.h"
#include "log.h"

#include <iostream>
#include <string>

Component::Component(const std::string& type, const Value& init) :
    _type(type)
{
    _address = init.get(1);
}

std::string Component::type() const
{
    return _type;
}

void Component::add(const std::string& methodName, ComponentMethod method)
{
    _methods[methodName] = method;
}

ValuePack Component::invoke(const std::string& methodName, const ValuePack& args)
{
    if (_methods.find(methodName) == _methods.end())
    {
        log << "component does not have any such method: " << methodName << std::endl;
        return ValuePack();
    }

    ComponentMethod pmethod = _methods.at(methodName);
    return ((*this).*pmethod)(args);
}

