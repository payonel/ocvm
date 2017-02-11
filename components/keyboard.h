#pragma once
#include "component.h"
#include "value.h"

class Keyboard : public Component
{
public:
    Keyboard(const Value& config, Host* host);
};
