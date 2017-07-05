#include "mouse_drv.h"
#include "term_buffer.h"

#include <iostream>
using std::cout;

unique_ptr<MouseEvent> MouseTerminalDriver::parse(TermBuffer* buffer)
{
    if (!buffer->hasMouseCode())
    {
        return nullptr; // ignore
    }

    // eat the mouse code header
    buffer->get();
    buffer->get();
    buffer->get();

    if (buffer->size() < 3)
        return nullptr; // ignore

    char b0 = buffer->get();
    char b1 = buffer->get();
    char b2 = buffer->get();

    EPressType press;
    int btn = b0 - 0x20;
    if (btn == 0x3)
    {
        if (_pressed == 0x3) // odd
        {
            return nullptr;
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
            return nullptr; // ignore drags if button state is released

        press = EPressType::Drag;
        _dragging = true;
        btn -= 0x20;
    }
    else // Press
    {
        if (_dragging || _pressed != 0x3) // ignore press if dragging or pressed
        {
            _pressed = btn; // but still update
            return nullptr;
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

    pm->x = (unsigned char)b1 - 32;
    pm->y = (unsigned char)b2 - 32;

    if (pm->x < 0)
        pm->x += 256;
    if (pm->y < 0)
        pm->y += 256;

    return unique_ptr<MouseEvent>(pm);
}
