#include "mouse_drv.h"
#include "term_buffer.h"

#include <iostream>
using std::cout;

vector<MouseEvent> MouseTerminalDriver::parse(TermBuffer* buffer)
{
    if (!buffer->hasMouseCode())
    {
        return {}; // ignore
    }

    // eat the mouse code header
    buffer->get();
    buffer->get();
    buffer->get();

    if (buffer->size() < 3)
        return {}; // ignore

    char b0 = buffer->get();
    char b1 = buffer->get();
    char b2 = buffer->get();

    EPressType press;
    int btn = b0 - 0x20;
    if (btn == 0x3)
    {
        if (_pressed == 0x3) // odd
        {
            return {};
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
            return {}; // ignore drags if button state is released

        press = EPressType::Drag;
        _dragging = true;
        btn -= 0x20;
    }
    else // Press
    {
        if (_dragging || _pressed != 0x3) // ignore press if dragging or pressed
        {
            _pressed = btn; // but still update
            return {};
        }

        press = EPressType::Press;
    }

    if (_pressed == 0x3)
        _pressed = btn;

    MouseEvent me;
    me.press = press;
    me.btn = _pressed;

    if (press != EPressType::Drag)
        _pressed = btn;

    me.x = (unsigned char)b1 - 32;
    me.y = (unsigned char)b2 - 32;

    if (me.x < 0)
        me.x += 256;
    if (me.y < 0)
        me.y += 256;

    return {me};
}
