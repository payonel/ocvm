#pragma once

#include "io/mouse_input.h"

class TermBuffer;
class MouseTerminalDriver
{
public:
    unique_ptr<MouseEvent> parse(TermBuffer* buffer);
private:
    int _pressed = 0x3;
    bool _dragging = false;
};
