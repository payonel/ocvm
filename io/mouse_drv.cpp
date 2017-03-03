#include "mouse_drv.h"
#include "input_drv.cpp"

MouseDriver::MouseDriver()
{
}

MouseDriver::~MouseDriver()
{
}

bool MouseDriver::isRunning()
{
    return false;
}

bool MouseDriver::start()
{
    return false;
}

void MouseDriver::stop()
{
}

bool MouseDriver::pop(MouseEvent* pme)
{
    return false;
}

void MouseDriver::enqueue()
{
}

