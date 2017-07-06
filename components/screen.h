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

    void push(KeyEvent ke) override;
    void push(MouseEvent me) override;
protected:
    bool onInitialize() override;
private:
    vector<Keyboard*> _keyboards;
    MouseInput* _mouse = nullptr;
};
