#pragma once

#include "input_drv.h"

#include <string>
#include <queue>
#include <thread>
#include <mutex>
using std::string;
using std::queue;
using std::thread;
using std::mutex;

struct MouseEvent
{
};

class MouseDriver : public InputDriver<MouseEvent>
{
public:
    MouseDriver();
    virtual ~MouseDriver();
    bool isRunning();

    bool start();
    void stop();
    bool pop(MouseEvent* pme);
protected:
    virtual void proc() = 0;
    // TODO enqueue should take just K_RAW stdin data
    // void enqueue(char* bytes, uint length);
    void enqueue();
    volatile bool _continue {};
private:
    mutex _m;
    queue<MouseEvent> _events;
    thread* _pthread {};
    bool _running {};
};

namespace Factory
{
    MouseDriver* create_mouse(const string& mouseTypeName);
};
