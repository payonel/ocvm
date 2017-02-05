#pragma once
#include "component.h"
#include "value.h"

class Gpu : public Component
{
public:
    Gpu(const std::string& type, const Value& init);

    ValuePack setResolution(const ValuePack& args);
    ValuePack bind(const ValuePack& args);
};
