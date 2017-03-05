#include "mouse_raw.h"

#include <iostream>
using namespace std;

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "worker.h"
#include "ansi.h"

class MouseLocalRawTtyDriverPrivate : public Worker
{
public:
    MouseLocalRawTtyDriverPrivate(MouseLocalRawTtyDriver* driver) :
        _driver(driver)
    {
    }
private:
    bool get(unsigned char* pOut)
    {
        struct timeval tv { 0L, 0L };
        fd_set fds;

        FD_ZERO(&fds);
        FD_SET(0, &fds);
        if (select(1, &fds, NULL, NULL, &tv))
        {
            int len = read(0, pOut, sizeof(unsigned char));
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

    void onStart() override
    {
        //save current settings
        _original = new termios;
        ::tcgetattr(STDIN_FILENO, _original);

        //put in raw mod
        termios raw;
        ::cfmakeraw(&raw);
        ::tcsetattr(STDIN_FILENO, TCSANOW, &raw);

        //enable mouse tracking
        cout << Ansi::mouse_prd_on;
    }

    bool runOnce() override
    {
        static unsigned char buf[3];
        if (get() == Ansi::ESC && get() == '[' && get() == 'M')
        {
            buf[0] = get();
            buf[1] = get();
            buf[2] = get();
            _driver->enqueue(buf);
        }

        return true;
    }

    void onStop() override
    {
        // disable mouse tracking
        cout << Ansi::mouse_prd_off;
        if (_original)
        {
            ::tcsetattr(STDIN_FILENO, TCSANOW, _original);
        }
        delete _original;
        _original = nullptr;
    }
    
    termios* _original {};
    MouseLocalRawTtyDriver* _driver;
};

MouseLocalRawTtyDriver::MouseLocalRawTtyDriver()
{
    _priv = new MouseLocalRawTtyDriverPrivate(this);
}

MouseLocalRawTtyDriver::~MouseLocalRawTtyDriver()
{
    _priv->stop();
    delete _priv;
    _priv = nullptr;
}

bool MouseLocalRawTtyDriver::onStart()
{
    return _priv->start();
}

void MouseLocalRawTtyDriver::onStop()
{
    _priv->stop();
}
