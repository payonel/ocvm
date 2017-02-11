#pragma once
#include "component.h"
#include "value.h"

class Gpu : public Component
{
public:
    Gpu(const Value& config, Host* host);

    ValuePack setResolution(const ValuePack& args);
    ValuePack bind(const ValuePack& args);
    ValuePack set(const ValuePack& args);
    ValuePack maxResolution(const ValuePack& args);
    ValuePack setBackground(const ValuePack& args);
    ValuePack getBackground(const ValuePack& args);
    ValuePack setForeground(const ValuePack& args);
    ValuePack getForeground(const ValuePack& args);
    ValuePack fill(const ValuePack& args);
};
