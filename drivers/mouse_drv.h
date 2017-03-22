#pragma once

#include "io/mouse_input.h"

class MouseDriverImpl : public MouseDriver
{
public:
    void enqueue(char b0, char b1, char b2);
private:
    int _pressed = 0x3;
    bool _dragging = false;
};
