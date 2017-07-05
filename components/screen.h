#pragma once
#include "component.h"
#include "model/value.h"
#include "io/frame.h"

class MouseInput;
class Keyboard;

class Screen : public Component, public Frame
{
public:
    Screen();
    ~Screen();
    RunState update() override;

    int getKeyboards(lua_State*);
    int getAspectRatio(lua_State*);

    bool connectKeyboard(Keyboard* kb);
    bool disconnectKeyboard(Keyboard* kb);
    vector<string> keyboards() const;

    void push(unique_ptr<KeyEvent> pke) override;
    void push(unique_ptr<MouseEvent> pme) override;
protected:
    bool onInitialize() override;
private:
    vector<Keyboard*> _keyboards;
    MouseInput* _mouse = nullptr;
};
