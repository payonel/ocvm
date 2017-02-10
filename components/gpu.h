#pragma once
#include "component.h"
#include "value.h"

class Gpu : public Component
{
public:
    Gpu(const string& type, const Value& init, Host* host);

    ValuePack setResolution(const ValuePack& args);
    ValuePack bind(const ValuePack& args);
    ValuePack set(const ValuePack& args);
};
