#include "input_drv.h"

#include <thread>
using namespace std;

template <typename TEventType>
InputDriver<TEventType>::InputDriver()
{
    // template hacks
    auto _start = &InputDriver<TEventType>::start;
    (void)_start;
    auto _pop = &InputDriver<TEventType>::pop;
    (void)_pop;
    auto _on_start = &InputDriver<TEventType>::onStart;
    (void)_on_start;
    auto _on_stop = &InputDriver<TEventType>::onStop;
    (void)_on_stop;
    auto _push = &InputDriver<TEventType>::push;
    (void)_push;
}

template <typename TEventType>
InputDriver<TEventType>::~InputDriver()
{
    this->stop();
}

template <typename TEventType>
bool InputDriver<TEventType>::isRunning()
{
    return this->_pthread && this->_running;
}

template <typename TEventType>
bool InputDriver<TEventType>::start()
{
    if (this->isRunning())
        return false;

    this->_running = true;
    this->_continue = true;
    decltype(this->_events) empty_queue;
    std::swap(this->_events, empty_queue);
    this->_pthread = new thread(&InputDriver<TEventType>::proc, this);

    return true;
}

template <typename TEventType>
void InputDriver<TEventType>::stop()
{
    this->_continue = false;
    if (this->isRunning())
    {
        this->_pthread->join();
    }
    this->_running = false;

    delete this->_pthread;
    this->_pthread = nullptr;
}

template <typename TEventType>
bool InputDriver<TEventType>::pop(TEventType* pe)
{
    if (this->_events.size() == 0)
        return false;
    unique_lock<mutex> lk(this->_m);
    *pe = this->_events.front();
    this->_events.pop();
    return true;
}

template <typename TEventType>
void InputDriver<TEventType>::push(const TEventType& e)
{
    unique_lock<mutex> lk(this->_m);
    this->_events.push(e);
}

template <typename TEventType>
void InputDriver<TEventType>::proc()
{
    this->onStart();
    while (_continue)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
        this->runOnce();
    }
    this->onStop();
}

template <typename TEventType>
void InputDriver<TEventType>::onStart()
{
}

template <typename TEventType>
void InputDriver<TEventType>::onStop()
{
}
