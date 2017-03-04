#pragma once

#include <mutex>
#include <queue>
#include <thread>
#include <memory>
using std::mutex;
using std::queue;
using std::thread;
using std::unique_ptr;

struct InputEvent {};

class InputDriver
{
public:
    InputDriver();
    virtual ~InputDriver();
    bool isRunning();

    bool start();
    void stop();
    unique_ptr<InputEvent> pop();

protected:
    void push(unique_ptr<InputEvent> e);

    virtual void onStart();
    virtual bool runOnce() = 0;
    virtual void onStop();

private:
    void proc();
    volatile bool _continue = false;
    mutex _m;
    queue<InputEvent*> _events;
    thread* _pthread = nullptr;
    bool _running = false;
};
