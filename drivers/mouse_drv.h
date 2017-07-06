#pragma once

#include "io/mouse_input.h"
#include <vector>
using std::vector;

class TermBuffer;
class MouseTerminalDriver
{
public:
    vector<MouseEvent> parse(TermBuffer* buffer);
private:
    int _pressed = 0x3;
    bool _dragging = false;
};
