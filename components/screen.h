#pragma once
#include "component.h"
#include "value.h"
#include "framing/frame.h"

class Screen : public Component, public Frame
{
public:
    Screen();
    ValuePack getKeyboards(lua_State*);
protected:
    bool onInitialize(Value& config) override;
private:
    vector<string> _keyboards;
};
