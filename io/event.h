#pragma once

#include <vector>
#include <mutex>
#include <queue>
#include <memory>
using std::mutex;
using std::queue;
using std::unique_lock;
using std::vector;

enum class EPressType
{
    Press,
    Release,
    Drag,
    Drop
};

struct MouseEvent
{
    EPressType press;
    int x;
    int y;
    int btn;
};

struct KeyEvent
{
    unsigned int keysym;
    unsigned int keycode;
    bool bPressed; // false: released

    bool bShift;    //0x01
    bool bCaps;     //0x02
    bool bControl;  //0x04
    bool bAlt;      //0x08
    bool bNumLock;  //0x10

    std::vector<char> insert;
};

struct ModemEvent
{
};

template <typename TEvent>
class EventSource
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

    void push(const TEvent& te)
    {
        unique_lock<mutex> lk(_m);
        _events.push(te);
    }
private:
    mutex _m;
    queue<TEvent> _events;
};
