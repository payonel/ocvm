#pragma once
#include "component.h"
#include "model/value.h"

class KeyboardInput;
class Screen;

class Keyboard : public Component
{
public:
    enum ConfigIndex
    {
        ScreenAddress = Component::ConfigIndex::Next
    };

    Keyboard();
    ~Keyboard();
    KeyboardInput* inputDevice() const;
    bool postInit() override;
    RunState update() override;
    void detach();
protected:
    bool onInitialize() override;
    Screen* screen() const;
private:
    string _preferredScreen;
    KeyboardInput* _keyboard = nullptr;
};
