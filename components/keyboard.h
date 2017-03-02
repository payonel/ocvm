#pragma once
#include "component.h"
#include "value.h"

class KeyboardDriver;

class Keyboard : public Component
{
public:
    Keyboard();
    ~Keyboard();
    bool postInit() override;
    RunState update() override;
protected:
    bool onInitialize(Value& config) override;
private:
    string _preferredScreen;
    KeyboardDriver* _driver {};
};
