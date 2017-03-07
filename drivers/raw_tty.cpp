#include "raw_tty.h"

#include <iostream>
#include <sstream>
#include <set>
using namespace std;

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>
#include <linux/kd.h>

#include "worker.h"
#include "ansi.h"

class TtyReader;
class RawTtyInputStreamImpl : public RawTtyInputStream
{
public:
    RawTtyInputStreamImpl(TtyReader* reader);
    unsigned char get() override;
    RawTtyInputStreamImpl* reset();
    void clear();
private:
    vector<unsigned char> _buffer;
    TtyReader* _reader;
    size_t _index;
};

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

    bool hasTerminalOut() const
    {
        return _terminal_out;
    }

private:
    TtyReader()
    {
        _master_tty = false;
        _terminal_out = false;

        int ec = 0;
        ec = ioctl(0, KDGKBMODE, &_original_kb_mode);
        if (ec == 0) // success
        {
            ec = ioctl(0, KDSKBMODE, _original_kb_mode);
        }
        _master_tty = ec == 0 && errno == 0;

        _terminal_out = !(isatty(fileno(stdout)));
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
        RawTtyInputStreamImpl stream(this);
        unsigned char c = stream.get();
        if (c == Ansi::ESC)
        {
            if (stream.get() == '[' && stream.get() == 'M')
            {
                stream.clear(); // clear the buffer thus far, the 3 bytes
                for (auto driver : _mouse_drivers)
                    driver->enqueue(stream.reset());
            }
        }
        else if (c > 0)
        {
            for (auto driver : _kb_drivers)
                driver->enqueue(stream.reset());
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
    bool _terminal_out;
    static unsigned long _original_kb_mode;
    termios* _original = nullptr;
    set<MouseLocalRawTtyDriver*> _mouse_drivers;
    set<KeyboardLocalRawTtyDriver*> _kb_drivers;
};
unsigned long TtyReader::_original_kb_mode = 0;

RawTtyInputStreamImpl::RawTtyInputStreamImpl(TtyReader* reader) :
    _reader(reader),
    _index(0)
{
}

unsigned char RawTtyInputStreamImpl::get()
{
    while (_index >= _buffer.size())
    {
        unsigned char next = _reader->get();
        if (next == 0)
            return next;

        _buffer.push_back(_reader->get());
    }

    unsigned next = _buffer.at(_index++);
    return next;
}

RawTtyInputStreamImpl* RawTtyInputStreamImpl::reset()
{
    _index = 0;
    return this;
}

void RawTtyInputStreamImpl::clear()
{
    _buffer.clear();
}

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

void MouseLocalRawTtyDriver::enqueue(RawTtyInputStream* stream)
{
    unsigned char buf[] {stream->get(), stream->get(), stream->get()};
    MouseDriverImpl::enqueue(buf);
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

bool MouseLocalRawTtyDriver::isAvailable()
{
    return TtyReader::engine()->hasTerminalOut();
}

void KeyboardLocalRawTtyDriver::enqueue(RawTtyInputStream* stream)
{
    unsigned char keycode = stream->get();
    bool bPressed = keycode & 0x80;

    stringstream ss;
    ss << (int)keycode;

    KeyboardDriverImpl::enqueue(bPressed, ss.str(), 2, 1, 3, 0);
}
