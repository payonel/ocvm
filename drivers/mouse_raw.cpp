#include "mouse_raw.h"

class MouseLocalRawTtyDriverPrivate
{
};

MouseLocalRawTtyDriver::MouseLocalRawTtyDriver()
{
    _priv = new MouseLocalRawTtyDriverPrivate;
}

MouseLocalRawTtyDriver::~MouseLocalRawTtyDriver()
{
    delete _priv;
    _priv = nullptr;
}

void MouseLocalRawTtyDriver::proc()
{
}
