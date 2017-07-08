#pragma once
#include "component.h"
#include "model/value.h"
#include "io/event.h"

class Screen;

class Keyboard : public Component, public EventSource<KeyEvent>
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
    void detach();
protected:
    bool onInitialize() override;
    Screen* screen() const;
private:
    string _preferredScreen;
};
