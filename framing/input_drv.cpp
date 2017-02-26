#include "input_drv.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <ctype.h>

#include <queue>
#include <thread>
#include <mutex>
using namespace std;

class _KeyboardDriver
{
public:
    _KeyboardDriver() :
        _pthread(nullptr),
        _running(false),
        _continue(false)
    {
    }

    ~_KeyboardDriver()
    {
        stop();
    }

    void stop()
    {
        _continue = false;
        if (isRunning())
        {
            _pthread->join();
        }
        _running = false;

        delete _pthread;
        _pthread = nullptr;
    }

    bool start()
    {
        if (isRunning())
            return false;

        _running = true;
        _continue = true;
        decltype(_events) empty_queue;
        std::swap(_events, empty_queue);
        _pthread = new thread(&_KeyboardDriver::proc, this);

        return true;
    }

    bool isRunning()
    {
        return _pthread && _running;
    }

    bool pop(KeyEvent* pke)
    {
        if (_events.size() == 0)
            return false;
        unique_lock<mutex> lk(_m);
        *pke = _events.front();
        _events.pop();
        return true;
    }
protected:
    bool is_physical_release(Display* display, XEvent* pev)
    {
        if (pev && pev->type == KeyRelease)
        {
            if (XEventsQueued(display, QueuedAfterFlush))
            {
                XEvent pending;
                XPeekEvent(display, &pending);
                if (
                    pending.type == KeyPress &&
                    pending.xkey.time == pev->xkey.time &&
                    pending.xkey.keycode == pev->xkey.keycode)
                {
                    // ignore
                    return false;
                }
            }
        }
        return true;
    }


    void proc()
    {
        Display *display = XOpenDisplay(nullptr);
        Window active_window;
        int revert_to;

        XGetInputFocus(display, &active_window, &revert_to);
        XSelectInput(display, active_window, KeyPressMask | KeyReleaseMask);
        char buf[32] {};

        while (true)
        {
            XEvent event {};
            XNextEvent(display, &event);
            KeyEvent ke;

            if (event.type == KeyPress)
                ke.bPressed = true;
            else if (event.type == KeyRelease)
            {
                if (!is_physical_release(display, &event))
                    continue;
                ke.bPressed = false;
            }
            else
                continue;

            if (!_continue)
                break;

            buf[0] = 0;
            KeySym ks;
            XLookupString(&event.xkey, buf, sizeof(buf) - 1, &ks, nullptr);

            ke.text = buf;
            ke.keysym = keysymMap(ks);
            ke.keycode = event.xkey.keycode;

            ke.bShift = (event.xkey.state & 0x1);
            ke.bControl = (event.xkey.state & 0x4);
            ke.bAlt = (event.xkey.state & 0x8);

            {
                unique_lock<mutex> lk(_m);
                _events.push(ke);
            }
        }

        XCloseDisplay(display);
    }

private:
    thread* _pthread;
    mutex _m;
    bool _running;
    volatile bool _continue;
    queue<KeyEvent> _events;

    unsigned int keysymMap(KeySym& ks)
    {
        if (ks & 0xFF00)
        {
            return ks & 0xFF;
        }

        return ks;
    }

} kb_drv;

namespace InputDriver
{
    void stop()
    {
        kb_drv.stop();
    }

    bool start()
    {
        if (kb_drv.isRunning())
            return false;

        return kb_drv.start();
    }

    bool pop(KeyEvent* pke)
    {
        return kb_drv.pop(pke);
    }

    bool pop(MouseEvent* pme)
    {
        return false;
    }
};

