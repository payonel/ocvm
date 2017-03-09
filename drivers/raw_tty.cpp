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
        if (!hasTerminalOut())
            return; // ignore if we don't have an interactive shell

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
        if (!hasMasterTty())
            return; // ignore if we dont have master tty

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

    static inline bool has_input()
    {
        struct timeval tv { 0L, 0L };
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(0, &fds);
        return select(1, &fds, nullptr, nullptr, &tv);
    }

    static inline bool get(unsigned char* pOut)
    {
        return has_input() && read(0, pOut, sizeof(unsigned char)) > 0;
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

        _terminal_out = (isatty(fileno(stdout)));
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
        if (has_input())
        {
            RawTtyInputStreamImpl stream(this);
            unsigned char c = stream.get();
            if (c == Ansi::ESC && stream.get() == '[') // escape sequence
            {
                if (stream.get() == 'M') // mouse escape sequence
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

        _buffer.push_back(next);
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

KeyboardLocalRawTtyDriver::~KeyboardLocalRawTtyDriver()
{
    TtyReader::engine()->remove(this);
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

class RawKeyMap
{
public:
    RawKeyMap(RawKeyMap&) = delete;

    static RawKeyMap* get()
    {
        static RawKeyMap rkm;
        return &rkm;
    }

    uint keycode(RawTtyInputStream* reader, bool* preleased)
    {
        unsigned char c = reader->get();
        unsigned char byte_1 = 0x0;
        
        switch (c) // multi byte sequences
        {
            case 0xE0:
                byte_1 = reader->get();
                *preleased = byte_1 & 0x80;
                byte_1 &= 0x7F;
                switch (byte_1)
                {
                    case 56: c = 108; break; // ralt
                    case 71: c = 110; break; // home
                    case 73: c = 112; break; // pg up
                    case 72: c = 111; break; // up
                    case 75: c = 113; break; // left
                    case 77: c = 114; break; // right
                    case 79: c = 115; break; // end
                    case 80: c = 116; break; // down
                    case 81: c = 117; break; // pg dn
                    case 82: c = 118; break; // insert
                    case 83: c = 119; break; // delete
                    case 91: c = 133; break; // windows
                    case 93: c = 135; break; // menu
                    default: c = byte_1; break;
                }
                break;
            case 0xE1: 
                byte_1 = reader->get(); // 29
                *preleased = byte_1 & 0x80;
                reader->get(); // 69 
                c = 127;
                break;
            default:
                *preleased = c & 0x80;
                c &= 0x7F;
                switch (c)
                {
                    case 29: c =  37; break; // left control
                    case 56: c =  64; break; // left alt
                    case 91: c = 133; break; // windows
                }
                break;
        }

        return c;
    }
private:
    RawKeyMap()
    {
    }
};

void KeyboardLocalRawTtyDriver::enqueue(RawTtyInputStream* stream)
{
    bool bReleased;
    uint keycode = RawKeyMap::get()->keycode(stream, &bReleased);
    bool bPressed = !bReleased;

    KeyboardDriverImpl::enqueue(bPressed, keycode);
}
