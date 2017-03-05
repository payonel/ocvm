#include "mouse_drv.h"

#include <iostream>
using std::cout;

void MouseDriverImpl::enqueue(unsigned char* buf)
{
    EPressType press;
    int btn = buf[0] - 0x20;
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

    pm->x = buf[1] - 32;
    pm->y = buf[2] - 32;
    _source->push(std::move(unique_ptr<MouseEvent>(pm)));
}
