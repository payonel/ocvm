#include "input_drv.h"

#include <thread>
using namespace std;

InputDriver::InputDriver()
{
}

InputDriver::~InputDriver()
{
    this->stop();
}

bool InputDriver::isRunning()
{
    return this->_pthread && this->_running;
}

bool InputDriver::start()
{
    if (this->isRunning())
        return false;

    this->_running = true;
    this->_continue = true;
    decltype(this->_events) empty_queue;
    std::swap(this->_events, empty_queue);
    this->_pthread = new thread(&InputDriver::proc, this);

    return true;
}

void InputDriver::stop()
{
    this->_continue = false;
    if (this->isRunning())
    {
        this->_pthread->join();
    }
    this->_running = false;

    delete this->_pthread;
    this->_pthread = nullptr;

    while (pop());
}

unique_ptr<InputEvent> InputDriver::pop()
{
    if (this->_events.size() == 0)
        return {}; // empty
    unique_lock<mutex> lk(this->_m);
    unique_ptr<InputEvent> pe(this->_events.front());
    this->_events.pop();
    return pe;
}

void InputDriver::push(unique_ptr<InputEvent> pe)
{
    unique_lock<mutex> lk(this->_m);
    this->_events.push(pe.release());
}

void InputDriver::proc()
{
    this->onStart();
    while (_continue)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        if (!this->runOnce())
            break;
    }
    this->onStop();
}

void InputDriver::onStart()
{
}

void InputDriver::onStop()
{
}
