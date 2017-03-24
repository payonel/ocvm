#pragma once
#include "component.h"
#include "value.h"
#include "io/frame.h"

class MouseInput;

class Screen : public Component, public Frame
{
public:
    Screen();
    ~Screen();
    RunState update() override;

    int getKeyboards(lua_State*);
    void addKeyboard(const string& addr);
protected:
    bool onInitialize(Value& config) override;
    void onResize(int width, int height) override;
private:
    vector<string> _keyboards;
    MouseInput* _mouse = nullptr;
};
