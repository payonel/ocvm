#include "mouse_drv.h"

#include <iostream>
using std::cout;

void MouseDriverImpl::enqueue(unsigned char* buf)
{
    unsigned char btn = buf[0] - 32;
    EPressType press;
    if (btn == 0x3) // release, don't report
    {
        if (!_pressed)
            return; // ignore drops without press
        if (_dragging)
            press = EPressType::Drop;
        else
            press = EPressType::Release;
        _pressed = false;
        _dragging = false;
    }
    else if (btn < 0x20) //32
    {
        _pressed = true;
        if (_dragging)
            return;
        press = EPressType::Press;
    }
    else // drag
    {
        press = EPressType::Drag;
        if (!_pressed)
            return; // not dragging if no buttons pressed

        _dragging = true;
    }

    MouseEvent* pm = new MouseEvent;
    pm->press = press;
    pm->x = buf[1] - 32;
    pm->y = buf[2] - 32;
    _source->push(std::move(unique_ptr<MouseEvent>(pm)));
}
