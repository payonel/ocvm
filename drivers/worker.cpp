#include "worker.h"

#include <thread>

void NiceWork::set()
{
    _work_done = true;
}

NiceWork::~NiceWork()
{
    if (!_work_done)
        std::this_thread::sleep_for(std::chrono::milliseconds(1000/20));
}

Worker::~Worker()
{
}

bool Worker::isRunning()
{
    return _pthread && _running;
}

bool Worker::start()
{
    make_lock();
    if (isRunning())
        return false;

    _running = true;
    _continue = true;
    _pthread = new thread(&Worker::proc, this);

    return true;
}

void Worker::stop()
{
    {
        make_lock();
        _continue = false;
    }

    if (_pthread)
        _pthread->join();

    make_lock();
    _running = false;
    delete _pthread;
    _pthread = nullptr;
}

void Worker::proc()
{
    {
        make_lock();
        _continue = _continue && onStart();
    }
    while (_continue)
    {
        {
            make_lock();
            if (!runOnce())
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    make_lock();
    onStop();
    _running = false;
}

unique_lock<mutex> Worker::make_lock()
{
    return std::move(unique_lock<mutex> (_m));
}
