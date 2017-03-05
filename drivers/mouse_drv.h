#pragma once

#include "io/mouse_input.h"

class MouseDriverImpl : public MouseDriver
{
public:
    void enqueue(unsigned char* buf);
private:
    bool _pressed = false;
    bool _dragging = false;
};
