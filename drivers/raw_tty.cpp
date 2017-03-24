#include "raw_tty.h"

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#ifdef __linux__
#include <linux/kd.h>
#endif

#include <signal.h>
#include <string.h> // memset

#include "worker.h"
#include "ansi.h"

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
    exit(0);
}

class TtyReader : public Worker
{
public:
    TtyReader(TtyReader&) = delete;
    void operator= (TtyReader&) = delete;

    void add(TermInputDriver* driver)
    {
        bool first = false;
        {
            auto lk = make_lock();
            first = _drivers.size() == 0;
            _drivers.insert(driver);
        }
        if (first)
            start();
    }

    void remove(TermInputDriver* driver)
    {
        bool last = false;
        {
            auto lk = make_lock();
            _drivers.erase(driver);
            last = _drivers.size() == 0;
        }
        if (last)
            stop();
    }

    static TtyReader* engine()
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

    static inline bool get(char* pOut)
    {
        struct timeval tv { 0L, 0L };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        if (select(1, &fds, nullptr, nullptr, &tv))
        {
            return read(0, pOut, sizeof(char)) > 0;
        }
        
        return false;
    }

    void flush_stdin()
    {
        char byte;
        while (get(&byte))
        {
            _buffer.push(byte);
        }
    }

    bool hasMasterTty() const
    {
        return _master_tty;
    }

    bool hasTerminalOut() const
    {
        return _terminal_out;
    }

private:
    TtyReader()
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


    void onStart() override
    {
        //save current settings
        _original = new termios;
        ::tcgetattr(STDIN_FILENO, _original);

        //put in raw mod
        termios raw;
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
                cerr << "critical failure: tried to use ioctl\n";
                ::exit(1);
            }

            atexit(&exit_function);
        }
#endif

        cout << flush;
    }

    bool runOnce() override
    {
        flush_stdin();
        auto old_size = _buffer.size();
        if (old_size > 0)
        {
            for (auto driver : _drivers)
                driver->enqueue(&_buffer);
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

    void onStop() override
    {
#ifdef __linux__
        // leave raw mode
        ioctl(0, KDSKBMODE, _original_kb_mode);
#endif
        if (_original)
        {
            ::tcsetattr(STDIN_FILENO, TCSANOW, _original);
        }
        delete _original;
        _original = nullptr;
    }
    
    bool _master_tty;
    bool _terminal_out;
    termios* _original = nullptr;
    set<TermInputDriver*> _drivers;
    TermBuffer _buffer;
};
void MouseTerminalDriver::enqueue(TermBuffer* buffer)
{
    if (!buffer->hasMouseCode())
    {
        return; // ignore
    }
    // eat the mouse code header
    buffer->get();
    buffer->get();
    buffer->get();

    MouseDriverImpl::enqueue(buffer);
}

bool KeyboardLocalRawTtyDriver::isAvailable()
{
    return TtyReader::engine()->hasMasterTty();
}

bool MouseTerminalDriver::isAvailable()
{
    return TtyReader::engine()->hasTerminalOut();
}

void KeyboardLocalRawTtyDriver::enqueue(TermBuffer* buffer)
{
    if (buffer->hasMouseCode())
        return;

    bool released;
    unsigned int keycode = buffer->get();

    switch (keycode)
    {
        case 0xE0: // double byte
            keycode = buffer->get();
            released = keycode & 0x80;
            keycode |= 0x80; // add press indicator
            break;
        case 0xE1: // triple byte
            keycode = buffer->get(); // 29(released) or 29+0x80[157](pressed)
            released = keycode & 0x80;
            // NUMLK is a double byte 0xE0, 69 (| x80)
            // PAUSE is a triple byte 0xE1, 29 (| x80), 69 (| 0x80)
            // because triple byte press state is encoded in the 2nd byte
            // the third byte should retain 0x80
            keycode = buffer->get() | 0x80;
            break;
        default:
            released = keycode & 0x80;
            keycode &= 0x7F; // remove pressed indicator
            break;
    }

    KeyboardDriverImpl::enqueue(!released, keycode);
}

void KeyboardPtyDriver::enqueue(TermBuffer* buffer)
{
    if (buffer->hasMouseCode())
        return;

    KeyboardDriverImpl::enqueue(buffer);
}

bool KeyboardPtyDriver::isAvailable()
{
    return true;
}

TermInputDriver::~TermInputDriver()
{
    TtyReader::engine()->remove(this);
}

bool TermInputDriver::onStart()
{
    TtyReader::engine()->add(this);
    return true;
}

void TermInputDriver::onStop()
{
    TtyReader::engine()->remove(this);
}

bool MouseTerminalDriver::onStart()
{
    if (TermInputDriver::onStart())
    {
        if (TtyReader::engine()->hasTerminalOut())
        {
            cout << Ansi::mouse_prd_on << flush;
        }
        return true;
    }
    return false;
}

void MouseTerminalDriver::onStop()
{
    TermInputDriver::onStop();
    cout << Ansi::mouse_prd_off << flush;
}
