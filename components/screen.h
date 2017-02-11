#pragma once
#include "component.h"
#include "value.h"

class Screen : public Component
{
public:
    Screen(const Value& config, Host* host);

    ValuePack getKeyboards(const ValuePack& args);
private:
    vector<string> _keyboards;
};
