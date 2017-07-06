#pragma once

#include <mutex>
#include <queue>
#include <memory>
using std::mutex;
using std::queue;
using std::unique_lock;

template <typename TEvent>
class InputSource
{
public:
    bool pop(TEvent& te)
    {
        unique_lock<mutex> lk(_m);
        if (_events.size() == 0)
            return false; // empty
        te = _events.front();
        _events.pop();
        return true;
    }

    void push(TEvent te)
    {
        unique_lock<mutex> lk(_m);
        _events.push(te);
    }
private:
    mutex _m;
    queue<TEvent> _events;
};
