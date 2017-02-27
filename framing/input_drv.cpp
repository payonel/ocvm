#include "input_drv.h"

#include <X11/X.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <stdio.h>
#include <ctype.h>

#include <queue>
#include <thread>
#include <mutex>
#include <map>
using namespace std;

class _KeyboardDriver
{
public:
    _KeyboardDriver() :
        _pthread(nullptr),
        _running(false),
        _continue(false)
    {
        // [9, 96] = x-8
        for (uint src = 9; src <= 96; src++)
        {
            _codes[src] = src - 8;
        }
        // [133, 137] = x+86
        for (uint src = 133; src <= 137; src++)
        {
            _codes[src] = src + 86;
        }

        _codes[105] = 157;
        _codes[108] = 184;

        _codes[127] = 197;

        _codes[110] = 199;
        _codes[111] = 200;
        _codes[112] = 201;
        _codes[113] = 203;
        _codes[114] = 205;
        _codes[115] = 207;
        _codes[116] = 208;
        _codes[117] = 209;
        _codes[118] = 210;
        _codes[119] = 211;
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
            int len = XLookupString(&event.xkey, buf, sizeof(buf) - 1, &ks, nullptr);

            ke.text = buf;
            ke.keysym = map_sym(ks, len);
            ke.keycode = map_code(event.xkey.keycode);

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

    map<uint, uint> _codes;

    uint map_code(const uint& code)
    {
        const auto& it = _codes.find(code);
        if (it != _codes.end())
        {
            return it->second;
        }

        return code;
    }

    uint map_sym(const KeySym& sym, int sequence_length)
    {
        if (sequence_length == 0)
            return 0;

        if (sym & 0xFF00)
            return sym & 0x7F;

        return sym;
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

