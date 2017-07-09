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
    return true;
}

bool ModemDriver::send(const vector<char>& payload)
{
    _source->push({payload});
    return true;
}

bool ModemDriver::start()
{
    return true;
}

void ModemDriver::stop()
{
}
