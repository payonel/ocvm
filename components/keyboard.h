#pragma once
#include "component.h"
#include "value.h"

class Keyboard : public Component
{
public:
    Keyboard(const string& type, const Value& init, Host* host);
};
