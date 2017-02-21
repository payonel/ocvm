#include "ansi_escape.h"
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

#include <stdio.h>
#include <unistd.h>
#include <termios.h>

static const string esc = "\033[";
static const string cursor_on  = esc + "?25h";
static const string cursor_off = esc + "?25l";
static const string track_on   = esc + "?1002h";
static const string track_off  = esc + "?1002l";
static const string mouse_on   = esc + "?9h";
static const string mouse_off  = esc + "?9l";
static const string clear_term = esc + "2J";
static const string clean_line = esc + "K";
static const string save_pos   = esc + "s";
static const string restore_pos= esc + "u";

bool kbhit()
{
    struct timeval tv { 0L, 0L };
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(0, &fds);
    return select(1, &fds, NULL, NULL, &tv);
}

int getch()
{
    int r;
    unsigned char c;
    if ((r = read(0, &c, sizeof(c))) < 0)
    {
        return r;
    }
    else
    {
        return c;
    }
}

string set_pos(int x, int y)
{
    stringstream ss;
    ss << esc << y << ";" << x << "f";
    return ss.str();
}

void refresh()
{
    cout << flush;
}

void AnsiEscapeTerm::print(AnsiFrameState* pf, int x, int y, const string& text)
{
    cout << set_pos(x, y);
    cout << text;
}

AnsiEscapeTerm::~AnsiEscapeTerm()
{
    close();
}

bool AnsiEscapeTerm::update()
{
    Frame* pActiveFrame = nullptr;
    for (const auto& pf : _frames)
    {
        auto& state = _states[pf];
        while (!pf->empty())
        {
            auto buffer = pf->pop();
            string text = std::get<2>(buffer);

            if (pf->scrolling())
            {
                // log to file
                ofstream flog("log", fstream::app);
                flog << text;
                flog.close();
            }
            else
            {
                int x = std::get<0>(buffer) + 1;
                int y = std::get<1>(buffer) + 1;
                print(&state, x, y, text);
            }
        }
        if (!pf->scrolling())
        // if (pf->isActive()) ?
            pActiveFrame = pf;
    }
    refresh();

    if (kbhit())
    {
        int ch = getch();
        if (ch == 3)
        {
            cerr << "shell abort\n";
            return false;
        }
        else if (ch == '\x1B')
        {
            cin.get();
            cin.get(); // throw away? what are these
            int btn = cin.get() - 32;
            int x = cin.get() - 32;
            int y = cin.get() - 32;
            if (pActiveFrame)
            {
                pActiveFrame->mouse(btn, x, y);
            }
        }
        else // assume char?
        {
            if (pActiveFrame)
            {
                pActiveFrame->keyboard(ch);
            }
        }
    }

    return true;
}

bool AnsiEscapeTerm::open()
{
    if (_original)
    {
        cerr << "terminal is already open\n";
        return false;
    }

    //save current settings
    _original = new termios;
    ::tcgetattr(STDIN_FILENO, _original);

    cout << cursor_off;

    //put in raw mod
    termios raw;
    ::cfmakeraw(&raw);
    ::tcsetattr(STDIN_FILENO, TCSANOW, &raw);

    //switch to alternative buffer screen
    // cout << esc << "47h";

    //enable mouse tracking
    cout << mouse_on;

    cout << clear_term << set_pos(1, 1) << flush;
    return true;
}

void AnsiEscapeTerm::close()
{
    // disable mouse tracking
    cout << mouse_off;
    cout << cursor_on;
    if (_original)
    {
        ::tcsetattr(STDIN_FILENO, TCSANOW, _original);
    }
    delete _original;
    _original = nullptr;
    _states.clear();
    cout << set_pos(1, 1);
    cout << flush;
}

bool AnsiEscapeTerm::onAdd(Frame* pf)
{
    // WINDOW* pwin = newwin(0, 0, 0, 0);
    // scrollok(pwin, pf->scrolling());
    _states[pf] = {}; // full screen window
    // onResolution(pf);
    return true;
}

void AnsiEscapeTerm::onWrite(Frame* pWhichFrame)
{
}

void AnsiEscapeTerm::onResolution(Frame* pWhichFrame)
{
}

tuple<int, int> AnsiEscapeTerm::maxResolution() const
{
    return std::make_tuple(80, 25);
}
