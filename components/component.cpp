#include "component.h"
#include "log.h"

#include <random>

#include <iostream>
#include <string>
#include <vector>
#include <sstream>
using std::string;
using std::endl;
using std::vector;
using std::stringstream;

Component::Component(const string& type, const Value& init) :
    _type(type)
{
    Value v;
    v = init.get(1);
    if (!v || v.type() != "string" || v.toString().empty())
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
        lout << "component[" << type() << "] does not have any such method: " << methodName << endl;
        return ValuePack();
    }

    ComponentMethod pmethod = _methods.at(methodName);
    return ((*this).*pmethod)(args);
}

string Component::make_address()
{
    // -- e.g. 3c44c8a9-0613-46a2-ad33-97b6ba2e9d9a
    // -- 8-4-4-4-12 (halved sizes because bytes make hex pairs)
    vector<int> sets {4, 2, 2, 2, 6};
    string result = "";

    for (auto len : sets)
    {
        if (!result.empty())
            result += "-";
        for (int index = 0; index < len; index++)
        {
            stringstream ss;
            ss << std::hex;
            ss << int(rand() % 256);
            result += ss.str();
        }
    }

    return result;
}
