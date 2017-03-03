#pragma once

#include <mutex>
#include <queue>
#include <thread>
using std::mutex;
using std::queue;
using std::thread;

template <typename TEventType>
class InputDriver
{
public:
    InputDriver();
    virtual ~InputDriver();
    bool isRunning();

    bool start();
    void stop();
    bool pop(TEventType* pe);

protected:
    void push(const TEventType& e);

    virtual void onStart();
    virtual bool runOnce() = 0;
    virtual void onStop();

private:
    void proc();
    volatile bool _continue = false;
    mutex _m;
    queue<TEventType> _events;
    thread* _pthread = nullptr;
    bool _running = false;
};
