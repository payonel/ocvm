#pragma once
#include "component.h"
#include "value.h"

class Filesystem : public Component
{
public:
    Filesystem(const string& type, const Value& init, Host* host);
};
