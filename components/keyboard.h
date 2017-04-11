#pragma once
#include "component.h"
#include "model/value.h"

class KeyboardInput;

class Keyboard : public Component
{
public:
    enum ConfigIndex
    {
        ScreenAddress = Component::ConfigIndex::Next
    };

    Keyboard();
    ~Keyboard();
    bool postInit() override;
    RunState update() override;
protected:
    bool onInitialize() override;
private:
    string _preferredScreen;
    KeyboardInput* _keyboard = nullptr;
};
