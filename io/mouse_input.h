#pragma once

#include "input.h"

enum class EPressType
{
    Press,
    Release,
    Drag,
    Drop
};

struct MouseEvent
{
    EPressType press;
    int x;
    int y;
    int btn;
};

class MouseInput : public InputSource<MouseEvent>
{
};
