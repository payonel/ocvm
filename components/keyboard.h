#pragma once
#include "component.h"
#include "value.h"

class Keyboard : public Component
{
public:
    Keyboard();
protected:
    bool onInitialize(Value& config) override;
};
