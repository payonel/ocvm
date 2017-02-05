#pragma once
#include "component.h"
#include "value.h"

class Screen : public Component
{
public:
    Screen(const std::string& type, const Value& init);
};
