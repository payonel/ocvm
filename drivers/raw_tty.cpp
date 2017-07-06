#include "raw_tty.h"

#include <iostream>
#include <sstream>
#include <set>

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#ifdef __linux__
#include <linux/kd.h>
#include <sys/select.h>
#endif

#include <time.h>
#include <signal.h>
#include <string.h> // memset

#include "ansi.h"
#include "drivers/ansi_escape.h"

using std::set;
using std::cout;
using std::cerr;
using std::flush;

static unsigned long _original_kb_mode = 0;
static struct sigaction sig_action_data;

static inline void exit_function()
{
#ifdef __linux__
    // leave raw mode
    ioctl(0, KDSKBMODE, _original_kb_mode ? _original_kb_mode : K_UNICODE);
#endif
}

static void segfault_sigaction(int signal, siginfo_t* pSigInfo, void* arg)
{
    exit_function();
}

void TtyReader::start(AnsiEscapeTerm* pTerm)
{
    _pTerm = pTerm;

    if (hasMasterTty())
    {
        _kb_drv.reset(new KeyboardLocalRawTtyDriver);
    }
    else
    {
        _kb_drv.reset(new KeyboardPtyDriver);
    }
    if (hasTerminalOut())
    {
        cout << Ansi::mouse_prd_on << flush;
        _mouse_drv.reset(new MouseTerminalDriver);
    }

    Worker::start();
}

void TtyReader::stop()
{
    make_lock();
    _pTerm = nullptr;
    Worker::stop();
}

// static
TtyReader* TtyReader::engine()
{
    static TtyReader one;
    static bool init = false;
    if (!init)
    {
        init = true;
        memset(&sig_action_data, 0, sizeof(sig_action_data));
        sig_action_data.sa_sigaction = segfault_sigaction;
        sig_action_data.sa_flags = SA_SIGINFO;
        sigaction(SIGTERM, &sig_action_data, nullptr);
    }
    return &one;
}

bool TtyReader::hasMasterTty() const
{
    return _master_tty;
}

bool TtyReader::hasTerminalOut() const
{
    return _terminal_out;
}

TtyReader::TtyReader()
{
    _master_tty = false;
    _terminal_out = false;

#ifdef __linux__
    int ec = 0;
    ec = ioctl(0, KDGKBMODE, &_original_kb_mode);
    if (ec == 0) // success
    {
        ec = ioctl(0, KDSKBMODE, _original_kb_mode);
    }
    _master_tty = ec == 0 && errno == 0;
#endif

    _terminal_out = (isatty(fileno(stdout)));
}

void TtyReader::onStart()
{
    //save current settings
    _original = new termios;
    ::tcgetattr(STDIN_FILENO, _original);

    //put in raw mod
    termios raw;
    memset(&raw, 0, sizeof(termios));
    ::cfmakeraw(&raw);
    ::tcsetattr(STDIN_FILENO, TCSANOW, &raw);

#ifdef __linux__
    if (_master_tty)
    {
        int ec = ioctl(0, KDGKBMODE, &_original_kb_mode);
        ec = ioctl(0, KDSKBMODE, K_RAW);
        if (ec != 0 || errno != 0)
        {
            // try to reset kb JUST IN CASE
            ::ioctl(0, KDSKBMODE, K_UNICODE);
            ::tcsetattr(STDIN_FILENO, TCSANOW, _original);
            cerr << "critical failure: could not set raw mode\n";
            ::exit(1);
        }

        atexit(&exit_function);
    }
#endif

    cout << flush;
}

bool TtyReader::runOnce()
{
    while (true)
    {
        char byte = 0;
        struct timeval tv { 0L, 0L };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        if (select(1, &fds, nullptr, nullptr, &tv))
        {
            if (read(0, &byte, sizeof(char)) > 0)
            {
                _buffer.push(byte);
                continue;
            }
        }

        break;
    }

    auto old_size = _buffer.size();
    if (old_size > 0)
    {
        if (_mouse_drv && _pTerm)
        {
            auto vme = _mouse_drv->parse(&_buffer);
            for (const auto& me : vme)
                _pTerm->mouseEvent(me);
        }
        if (_kb_drv && _pTerm)
        {
            auto vke = _kb_drv->parse(&_buffer);
            for (const auto& ke : vke)
                _pTerm->keyEvent(ke);
        }

        if (old_size == _buffer.size()) // nothing could read the buffer
        {
            if (_buffer.hasMouseCode())
            {
                _buffer.get();
                _buffer.get();
                _buffer.get();
                _buffer.get();
                _buffer.get();
            }
            _buffer.get(); // pop one off
        }
    }

    return true;
}

void TtyReader::onStop()
{
#ifdef __linux__
    // leave raw mode
    ioctl(0, KDSKBMODE, _original_kb_mode);
#endif
    if (_original)
    {
        ::tcsetattr(STDIN_FILENO, TCSANOW, _original);
    }
    cout << Ansi::mouse_prd_off << flush;
    delete _original;
    _original = nullptr;
}
