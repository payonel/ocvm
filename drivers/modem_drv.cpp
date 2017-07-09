#include "modem_drv.h"

ModemDriver::ModemDriver(EventSource<ModemEvent>* source) :
    _source(source)
{
}

bool ModemDriver::close(int port)
{
    return false;
}

bool ModemDriver::open(int port)
{
    return false;
}

bool ModemDriver::isOpen(int port)
{
    return false;
}

bool ModemDriver::send(const vector<char>& payload)
{
    return false;
}

bool ModemDriver::start()
{
    return true;
}

void ModemDriver::stop()
{
}
