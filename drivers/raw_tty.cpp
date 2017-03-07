#include "raw_tty.h"

#include <iostream>
#include <set>
using namespace std;

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

#include "worker.h"
#include "ansi.h"

class TtyReader : public Worker
{
public:
    TtyReader(TtyReader&) = delete;
    void operator= (TtyReader&) = delete;

    size_t driver_count()
    {
        auto lk = make_lock();
        return _mouse_drivers.size() + _kb_drivers.size();
    }

    void add(MouseLocalRawTtyDriver* driver)
    {
        size_t size = driver_count();
        {
            auto lk = make_lock();
            _mouse_drivers.insert(driver);
        }
        if (size == 0)
            start();
    }

    void remove(MouseLocalRawTtyDriver* driver)
    {
        {
            auto lk = make_lock();
            _mouse_drivers.erase(driver);
        }
        if (driver_count() == 0)
            stop();
    }

    void add(KeyboardLocalRawTtyDriver* driver)
    {
        size_t size = driver_count();
        {
            auto lk = make_lock();
            _kb_drivers.insert(driver);
        }
        if (size == 0)
            start();
    }

    void remove(KeyboardLocalRawTtyDriver* driver)
    {
        {
            auto lk = make_lock();
            _kb_drivers.erase(driver);
        }
        if (driver_count() == 0)
            stop();
    }

    static TtyReader* engine()
    {
        static TtyReader one;
        return &one;
    }

    bool get(unsigned char* pOut)
    {
        struct timeval tv { 0L, 0L };
        fd_set fds;

        FD_ZERO(&fds);
        FD_SET(0, &fds);
        if (select(1, &fds, NULL, NULL, &tv))
        {
            int len = read(0, pOut, sizeof(unsigned char));
            if (len)
            {
                cerr << (int)(*pOut) << "\r\n";
            }
            return len > 0;
        }

        return false;
    }

    unsigned char get()
    {
        unsigned char c;
        if (get(&c))
            return c;
        return static_cast<unsigned char>(0);
    }

    bool hasMasterTty() const
    {
        return _master_tty;
    }

private:
    TtyReader()
    {
        _master_tty = false;

        int ec = 0;
        ec = ioctl(0, KDGKBMODE, &_original_kb_mode);
        if (ec == 0) // success
        {
            ec = ioctl(0, KDSKBMODE, _original_kb_mode);
        }
        _master_tty = ec == 0 && errno == 0;
    }

    static void exit_function()
    {
        // leave raw mode
        ioctl(0, KDSKBMODE, _original_kb_mode ? _original_kb_mode : K_UNICODE);
    }

    void onStart() override
    {
        //save current settings
        _original = new termios;
        ::tcgetattr(STDIN_FILENO, _original);

        //put in raw mod
        termios raw;
        ::cfmakeraw(&raw);
        ::tcsetattr(STDIN_FILENO, TCSANOW, &raw);

        if (_master_tty)
        {
            int ec = ioctl(0, KDGKBMODE, &_original_kb_mode);
            ec = ioctl(0, KDSKBMODE, K_RAW);
            if (ec != 0 || errno != 0)
            {
                // try to reset kb JUST IN CASE
                ::ioctl(0, KDSKBMODE, K_UNICODE);
                ::tcsetattr(STDIN_FILENO, TCSANOW, _original);
                cerr << "critical failure: tried to use ioctl\n";
                ::exit(1);
            }
            atexit(&exit_function);
        }

        //enable mouse tracking
        cout << Ansi::mouse_prd_on;
        cout << flush;
    }

    bool runOnce() override
    {
        static unsigned char buf[3];
        unsigned char c = get();
        if (c == Ansi::ESC)
        {
            if (get() == '[' && get() == 'M')
            {
                buf[0] = get();
                buf[1] = get();
                buf[2] = get();
                for (auto driver : _mouse_drivers)
                    driver->enqueue(buf);
            }
        }

        return true;
    }

    void onStop() override
    {
        // leave raw mode
        ioctl(0, KDSKBMODE, _original_kb_mode);

        // disable mouse tracking
        cout << Ansi::mouse_prd_off;
        if (_original)
        {
            ::tcsetattr(STDIN_FILENO, TCSANOW, _original);
        }
        delete _original;
        _original = nullptr;
    }
    
    bool _master_tty;
    static unsigned long _original_kb_mode;
    termios* _original = nullptr;
    set<MouseLocalRawTtyDriver*> _mouse_drivers;
    set<KeyboardLocalRawTtyDriver*> _kb_drivers;
};

unsigned long TtyReader::_original_kb_mode = 0;

class MouseLocalRawTtyDriverPrivate : public Worker
{
public:
    MouseLocalRawTtyDriverPrivate(MouseLocalRawTtyDriver* driver) :
        _driver(driver)
    {
    }
private:
    MouseLocalRawTtyDriver* _driver;
};

MouseLocalRawTtyDriver::MouseLocalRawTtyDriver()
{
}

MouseLocalRawTtyDriver::~MouseLocalRawTtyDriver()
{
    TtyReader::engine()->remove(this);
}

bool MouseLocalRawTtyDriver::onStart()
{
    TtyReader::engine()->add(this);
    return true;
}

void MouseLocalRawTtyDriver::onStop()
{
    TtyReader::engine()->remove(this);
}

KeyboardLocalRawTtyDriver::KeyboardLocalRawTtyDriver()
{
}

KeyboardLocalRawTtyDriver::~KeyboardLocalRawTtyDriver()
{
}

bool KeyboardLocalRawTtyDriver::onStart()
{
    TtyReader::engine()->add(this);
    return true;
}

void KeyboardLocalRawTtyDriver::onStop()
{
    TtyReader::engine()->remove(this);
}

bool KeyboardLocalRawTtyDriver::isAvailable()
{
    return TtyReader::engine()->hasMasterTty();
}
