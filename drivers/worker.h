#pragma once

#include <thread>
#include <mutex>
using std::thread;
using std::mutex;

class Worker
{
public:
    virtual ~Worker();
    bool isRunning();

    bool start();
    void stop();

protected:
    virtual void onStart() = 0;
    virtual bool runOnce() = 0;
    virtual void onStop() = 0;

private:
    void proc();
    volatile bool _continue = false;
    thread* _pthread = nullptr;
    bool _running = false;
    mutex _m;
};
