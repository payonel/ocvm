#pragma once

#include <memory>
#include <mutex>
#include <queue>
using std::unique_ptr;
using std::mutex;
using std::queue;

struct InputEvent {};
class InputSource;

class InputDriver
{
public:
    virtual ~InputDriver();
    bool start(InputSource*);
    void stop();
protected:
    virtual bool onStart() = 0;
    virtual void onStop() = 0;
    InputSource* _source = nullptr;
};

class InputSource
{
public:
    virtual ~InputSource();

    bool open(unique_ptr<InputDriver> pDriver);
    void close();

    unique_ptr<InputEvent> pop();
    void push(unique_ptr<InputEvent>);
private:
    mutex _m;
    queue<InputEvent*> _events;
    unique_ptr<InputDriver> _driver;
};
