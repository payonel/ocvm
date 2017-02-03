#pragma once
#include "component.h"

class Screen : public Component
{
public:
    void invoke(const std::string& methodName) override;
protected:
    void test();
};
