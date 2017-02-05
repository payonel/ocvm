#include "shell.h"
#include "frame.h"
#include "log.h"

#include <iostream>
#include <string>
#include <sstream>
using std::cout;
using std::endl;
using std::flush;
using std::string;
using std::stringstream;

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

string showCursor(bool enable)
{
    stringstream ss;
    ss << esc << "?25" << (enable ? "h" : "l");
    return ss.str();
}

string trackMouse(bool enable)
{
    stringstream ss;
    ss << esc << "?1002" << (enable ? "h" : "l");
    return ss.str();
}

string scrollOn(int top, int lines)
{
    stringstream ss;
    ss << esc << top << ";" << lines << "r";
    return ss.str();
}

string scrollOff()
{
    stringstream ss;
    ss << esc << "r";
    return ss.str();
}

string setPos(int x, int y)
{
    stringstream ss;
    ss << esc << y << ";" << x << "f";
    return ss.str();
}

string moveVertically(int num)
{
    stringstream ss;
    if (num == 0) return "";
    ss << esc << ::abs(num) << (num < 0 ? "B" : "A");
    return ss.str();
}

string moveHorizontally(int num)
{
    stringstream ss;
    if (num == 0) return "";
    ss << esc << ::abs(num) << (num < 0 ? "D" : "C");
    return ss.str();
}

string clear()
{
    stringstream ss;
    ss << esc << "2J";
    return ss.str();
}

string clearLine()
{
    stringstream ss;
    ss << esc << "K";
    return ss.str();
}

string savePos()
{
    stringstream ss;
    ss << esc << "s";
    return ss.str();
}

string restorePos()
{
    stringstream ss;
    ss << esc << "u";
    return ss.str();
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
            break;
        frame_index++;
    }

    if (frame_index >= _frames.size())
    {
        lout << "bad frame not found\n";
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

bool Shell::add(Frame* pf, size_t index)
{
    bool result = Framer::add(pf, index);
    if (result)
    {
        _states[pf] = {1, 1, 1, 1};
    }

    return result;
}

bool Shell::update()
{
    lout << "shell update\n";
    lout << "shell update\n";
    lout << "shell update\n";
    lout << "shell update\n";

    unsigned char buff [6];
    unsigned int x, y, btn;

    for (const auto& pf : _frames)
    {
        string buffer = pf->read();
        if (buffer.empty())
            continue;

        // first align this frame back to its correct location
        auto& state = _states[pf];
        int width, height;
        pf->getResolution(&width, &height);

        // if the region is scrolling, write to bottom of the region
        if (pf->scrolling())
        {
            buffer =
                setPos(state.left + state.x - 1, state.y + state.top + height - 1) +
                scrollOn(state.top, height) +
                buffer +
                scrollOff();
        }
        else
        {
            buffer = setPos(state.left + state.x - 1, state.y + state.top - 1) + buffer;
        }

        cout << buffer;
    }

    cout << flush;

    ::read(STDIN_FILENO, &buff, 1);
    if (buff[0] == 3)
    {
        // ^c
        lout << "shell abort\n";
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
        lout << "shell is already open\n";
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

    cout << clear();
    cout << setPos(1, 1);
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
