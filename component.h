#pragma once
#include <string>

class Component
{
public:
    virtual void invoke(const std::string& methodName) = 0;
};
