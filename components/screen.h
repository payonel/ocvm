#pragma once
#include "component.h"
#include "value.h"

class Screen : public Component
{
public:
    Screen();
    ValuePack getKeyboards(const ValuePack& args);
protected:
    bool onInitialize(Value& config) override;
private:
    vector<string> _keyboards;
};
