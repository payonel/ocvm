#pragma once

#include "io/mouse_input.h"

class TermBuffer;
class MouseDriverImpl : public MouseDriver
{
public:
    void enqueue(TermBuffer* buffer);
private:
    int _pressed = 0x3;
    bool _dragging = false;
};
