#include "worker.h"

#include <thread>

Worker::~Worker()
{
    stop();
}

bool Worker::isRunning()
{
    return _pthread && _running;
}

bool Worker::start()
{
    if (isRunning())
        return false;

    _running = true;
    _continue = true;
    _pthread = new thread(&Worker::proc, this);

    return true;
}

void Worker::stop()
{
    _continue = false;
    if (isRunning())
    {
        make_lock();
        _pthread->join();
    }
    _running = false;

    delete _pthread;
    _pthread = nullptr;
}

void Worker::proc()
{
    onStart();
    while (_continue)
    {
        {
            make_lock();
            if (!runOnce())
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    onStop();
}

unique_lock<mutex> Worker::make_lock()
{
    return std::move(unique_lock<mutex> (_m));
}
