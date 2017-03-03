#include "mouse_raw.h"

#include <iostream>
using namespace std;

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "ansi.h"

struct MouseLocalRawTtyDriverPrivate
{
    termios* _original {};
};

MouseLocalRawTtyDriver::MouseLocalRawTtyDriver()
{
    _priv = new MouseLocalRawTtyDriverPrivate;
}

MouseLocalRawTtyDriver::~MouseLocalRawTtyDriver()
{
    stop();
    delete _priv;
    _priv = nullptr;
}

void MouseLocalRawTtyDriver::onStart()
{
    //save current settings
    _priv->_original = new termios;
    ::tcgetattr(STDIN_FILENO, _priv->_original);

    //put in raw mod
    termios raw;
    ::cfmakeraw(&raw);
    ::tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    //enable mouse tracking
    cout << Ansi::mouse_on;
}

bool MouseLocalRawTtyDriver::runOnce()
{
    struct timeval tv { 0L, 0L };
    fd_set fds;
    unsigned char c;

    FD_ZERO(&fds);
    FD_SET(0, &fds);
    if (select(1, &fds, NULL, NULL, &tv))
    {
        read(0, &c, sizeof(c));
    }

    return true;
}

void MouseLocalRawTtyDriver::onStop()
{
    // disable mouse tracking
    cout << Ansi::mouse_off;
    if (_priv->_original)
    {
        ::tcsetattr(STDIN_FILENO, TCSANOW, _priv->_original);
    }
    delete _priv->_original;
    _priv->_original = nullptr;
}
