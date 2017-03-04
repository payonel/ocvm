#include "mouse_input.h"

bool MouseInput::open(unique_ptr<MouseDriver> driver)
{
    MouseDriver* pm = driver.release();
    unique_ptr<MouseDriver> pid(pm);
    return InputSource::open(std::move(pid));
}

unique_ptr<MouseEvent> MouseInput::pop()
{
    return unique_ptr<MouseEvent>(static_cast<MouseEvent*>(InputSource::pop().release()));
}
