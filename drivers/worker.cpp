#include "worker.h"

#include <thread>
using namespace std;

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
        unique_lock<mutex> lk(_m);
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
            unique_lock<mutex> lk(_m);
            if (!runOnce())
                break;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    onStop();
}
