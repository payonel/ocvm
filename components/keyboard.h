#pragma once
#include "component.h"
#include "value.h"

class Keyboard : public Component
{
public:
    Keyboard();
    bool postInit() override;
protected:
    bool onInitialize(Value& config) override;
private:
    string _preferredScreen;
};
