#pragma once
#include "component.h"
#include "value.h"

class Screen : public Component
{
public:
    Screen(const Value& value);
    void invoke(const std::string& methodName) override;
    void test();

    // 
};
