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

struct MouseEvent : public InputEvent
{
};

class MouseDriver : public InputDriver
{
public:
    MouseDriver();
    virtual ~MouseDriver();

protected:
    // TODO enqueue should take just K_RAW stdin data
    // void enqueue(char* bytes, uint length);
    void enqueue();
};

namespace Factory
{
    MouseDriver* create_mouse(const string& mouseTypeName);
};
