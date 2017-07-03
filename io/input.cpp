#include "input.h"

#include <thread>
using std::unique_lock;

InputSource::~InputSource()
{
    close();
}

bool InputSource::open(unique_ptr<InputDriver> pDriver)
{
    _driver = std::move(pDriver);
    return _driver && _driver->start(this);
}

void InputSource::close()
{
    if (_driver)
    {
        unique_lock<mutex> lk(_m);
        _driver->stop();
    }
    _driver.reset(nullptr);
}

unique_ptr<InputEvent> InputSource::pop()
{
    if (_events.size() == 0)
        return {}; // empty
    unique_lock<mutex> lk(_m);
    unique_ptr<InputEvent> pe(_events.front());
    _events.pop();
    return pe;
}

void InputSource::push(unique_ptr<InputEvent> pe)
{
    unique_lock<mutex> lk(_m);
    _events.push(pe.release());
}

InputDriver::~InputDriver()
{
}

bool InputDriver::start(InputSource* src)
{
    _source = src;
    return src && onStart();
}

void InputDriver::stop()
{
    onStop();
}
