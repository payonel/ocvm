#pragma once

#include "io/mouse_input.h"

class MouseDriverImpl : public MouseDriver
{
public:
    void enqueue(unsigned char* buf);
private:
    int _pressed = 0x3;
    bool _dragging = false;
};
