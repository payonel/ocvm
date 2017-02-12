#pragma once
#include "component.h"
#include "value.h"

class Surface;

class Gpu : public Component
{
public:
    Gpu();

    ValuePack setResolution(const ValuePack& args);
    ValuePack bind(const ValuePack& args);
    ValuePack set(const ValuePack& args);
    ValuePack maxResolution(const ValuePack& args);
    ValuePack setBackground(const ValuePack& args);
    ValuePack getBackground(const ValuePack& args);
    ValuePack setForeground(const ValuePack& args);
    ValuePack getForeground(const ValuePack& args);
    ValuePack fill(const ValuePack& args);
    ValuePack copy(const ValuePack& args);
protected:
    bool onInitialize(Value& config) override;
private:
    Surface* _surface;
};
