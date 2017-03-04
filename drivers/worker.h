#pragma once

#include <thread>
using std::thread;

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
};
