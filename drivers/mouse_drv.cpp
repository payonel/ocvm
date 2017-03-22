#include "mouse_drv.h"

#include <iostream>
using std::cout;

void MouseDriverImpl::enqueue(char b0, char b1, char b2)
{
    EPressType press;
    int btn = b0 - 0x20;
    if (btn == 0x3)
    {
        if (_pressed == 0x3) // odd
        {
            return;
        }

        if (_dragging)
        {
            press = EPressType::Drop;
        }
        else
        {
            press = EPressType::Release;
        }
        _dragging = false;
    }
    else if (btn >= 0x20)
    {        
        if (_pressed == 0x3)
            return; // ignore drags if button state is released

        press = EPressType::Drag;
        _dragging = true;
        btn -= 0x20;
    }
    else // Press
    {
        if (_dragging || _pressed != 0x3) // ignore press if dragging or pressed
        {
            _pressed = btn; // but still update
            return;
        }

        press = EPressType::Press;
    }

    if (_pressed == 0x3)
        _pressed = btn;

    MouseEvent* pm = new MouseEvent;
    pm->press = press;
    pm->btn = _pressed;

    if (press != EPressType::Drag)
        _pressed = btn;

    pm->x = b1 - 32;
    pm->y = b2 - 32;
    _source->push(std::move(unique_ptr<MouseEvent>(pm)));
}
