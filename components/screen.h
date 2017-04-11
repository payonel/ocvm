#pragma once
#include "component.h"
#include "model/value.h"
#include "io/frame.h"

class MouseInput;

class Screen : public Component, public Frame
{
public:
    Screen();
    ~Screen();
    RunState update() override;

    int getKeyboards(lua_State*);
    int getAspectRatio(lua_State*);

    void addKeyboard(const string& addr);
protected:
    bool onInitialize() override;
private:
    vector<string> _keyboards;
    MouseInput* _mouse = nullptr;
};
