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
        cout << Ansi::mouse_on;
    }

    bool runOnce() override
    {
        struct timeval tv { 0L, 0L };
        fd_set fds;
        unsigned char c;

        FD_ZERO(&fds);
        FD_SET(0, &fds);
        if (select(1, &fds, NULL, NULL, &tv))
        {
            read(0, &c, sizeof(c));
            _driver->enqueue();
        }

        return true;
    }

    void onStop() override
    {
        // disable mouse tracking
        cout << Ansi::mouse_off;
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
