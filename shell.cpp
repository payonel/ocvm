#include "shell.h"
#include "frame.h"
#include "log.h"

#include <iostream>
using std::cout;
using std::endl;
using std::flush;

#include <chrono>
#include <thread>

#include <stdio.h>
#include <unistd.h>
#include <termios.h>

static const char* esc = "\033[";
static constexpr int max_shell_size = 255 - 32;

Shell::Shell() :
    _original(nullptr)
{
}

Shell::~Shell()
{
    close();
}

void showCursor(bool enable)
{
    cout << esc << "?25" << (enable ? "h" : "l");
}

void trackMouse(bool enable)
{
    cout << esc << "?1002" << (enable ? "h" : "l");
}

void setPos(int x, int y)
{
    cout << esc << y << ";" << x << "f";
}

void moveVertically(int num)
{
    if (num == 0) return;
    cout << esc << ::abs(num) << (num < 0 ? "B" : "A");
}

void moveHorizontally(int num)
{
    if (num == 0) return;
    cout << esc << ::abs(num) << (num < 0 ? "D" : "C");
}

void clear()
{
    cout << esc << "2J";
}

void clearLine()
{
    cout << esc << "K";
}

void savePos()
{
    cout << esc << "s";
}

void restorePos()
{
    cout << esc << "u";
}

void Shell::onWrite(Frame* pWhichFrame)
{
}

void Shell::onResolution(Frame* pWhichFrame, int oldw, int oldh)
{
    int w, h;
    pWhichFrame->getResolution(&w, &h);
    int hdiff = h - oldh;

    if (w == oldw && h == oldh)
    {
        return;
    }

    size_t frame_index = 0;
    for (auto pf : _frames)
    {
        if (pf == pWhichFrame)
        {
            FrameState& state = _states.at(pWhichFrame);
            state.width = w;
            state.height = h;
            break;
        }
        frame_index++;
    }

    if (frame_index >= _frames.size())
    {
        log << "bad frame not found\n";
        return;
    }

    for (frame_index = frame_index + 1; frame_index < _frames.size(); frame_index++)
    {
        FrameState& state = _states.at(_frames.at(frame_index));
        state.top += hdiff;
    }
}

Frame* Shell::getFrame(int x, int y)
{
    if (x < 1 || y < 1 || x > 255 || y > 255)
        return nullptr;

    int top = 1;
    for (auto pf : _frames)
    {
        int width;
        int height;
        pf->getResolution(&width, &height);

        if (x <= width && y >= top && y < (height + top))
        {
            return pf;
        }
        top += height;
    }
    return nullptr;
}

bool Shell::add(Frame* pf)
{
    bool result = Framer::add(pf);
    if (result)
    {
        _states[pf] = {1, 1, 0, 0, 1, 1};
    }

    return result;
}

bool Shell::update()
{
    log << "shell update\n";

    unsigned char buff [6];
    unsigned int x, y, btn;

    cout << flush;

    ::read(STDIN_FILENO, &buff, 1);
    if (buff[0] == 3)
    {
        // ^c
        log << "shell abort\n";
        return false; // kill main loop
    }
    else if (buff[0] == '\x1B')
    {
        // this is assuming all escape sequences received are mouse coordinates
        ::read(STDIN_FILENO, &buff, 5);
        btn = buff[2] - 32;
        x = buff[3] - 32;
        y = buff[4] - 32;

        Frame* pf = getFrame(x, y);

        if (pf)
        {
            pf->mouse(x, y);
        }
    }

    return true;
}

bool Shell::open()
{
    if (_original)
    {
        log << "shell is already open\n";
        return false;
    }

    // save current settings
    _original = new termios;
    ::tcgetattr(STDIN_FILENO, _original);

    //put stdin in raw mode
    termios raw;
    ::cfmakeraw(&raw);
    ::tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    //switch to alternative buffer screen
    //::write(STDOUT_FILENO, "\e[?47h", 6);

    // //enable mouse tracking
    ::write(STDOUT_FILENO, "\e[?9h", 5);

    clear();
    setPos(1, 1);
    cout << flush;

    return true;
}

void Shell::close()
{
    if (!_original)
        return;

    // disable mouse tracking
    ::write(STDOUT_FILENO, "\e[?9l", 5);

    // screen buffer
    // ::write(STDOUT_FILENO, "\e[?47l", 6);

    // original attributes (we've had it in raw mode)
    ::tcsetattr(STDIN_FILENO, TCSANOW, _original);

    delete _original;
    _original = nullptr;
}
