#pragma once
#include "component.h"
#include "value.h"

class Keyboard : public Component
{
public:
    Keyboard(Value& config, Host* host);
};
