#include "ansi_escape.h"
#include "log.h"
#include <iostream>
#include <sstream>
#include <fstream>
using namespace std;

#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <sys/ioctl.h>

#include "ansi.h"

void refresh()
{
    cout << flush;
}

static void print(AnsiFrameState* pf, int x, int y, const Cell& cell)
{
    cout << Ansi::set_pos(x, y);
    cout << Ansi::set_color(cell.fg.rgb, cell.bg.rgb, 16777216);
    cout << cell.value;
    // cout << color_reset;
    cout << flush;
}

AnsiEscapeTerm::~AnsiEscapeTerm()
{
    close();
}

bool AnsiEscapeTerm::update()
{
    refresh();

    return true;
}

bool AnsiEscapeTerm::onOpen()
{
    //switch to alternative buffer screen
    // cout << esc << "47h";

    cout << Ansi::cursor_off;
    cout << Ansi::clear_term << Ansi::set_pos(1, 1) << flush;
    return true;
}

void AnsiEscapeTerm::onClose()
{
    cout << Ansi::cursor_on;
    _states.clear();
    cout << Ansi::set_pos(1, 1);
    cout << flush;
}

bool AnsiEscapeTerm::onAdd(Frame* pf)
{
    _states[pf] = {};
    return true;
}

void AnsiEscapeTerm::onWrite(Frame* pf, int x, int y, const Cell& cell)
{
    if (pf->scrolling())
    {
        ofstream flog("log", fstream::app);
        flog << cell.value;
        flog.close();
    }
    else
    {
        auto& state = _states[pf];
        print(&state, x, y, cell);
    }
}

void AnsiEscapeTerm::onResolution(Frame* pWhichFrame)
{
}

tuple<int, int> AnsiEscapeTerm::maxResolution() const
{
    // TODO: handle SIGWINCH for resize
    // see signals.h
    int cols = 80;
    int lines = 24;

#ifdef TIOCGSIZE
    struct ttysize ts;
    ioctl(STDIN_FILENO, TIOCGSIZE, &ts);
    cols = ts.ts_cols;
    lines = ts.ts_lines;
#elif defined(TIOCGWINSZ)
    struct winsize ts;
    ioctl(STDIN_FILENO, TIOCGWINSZ, &ts);
    cols = ts.ws_col;
    lines = ts.ws_row;
#endif /* TIOCGSIZE */

    return std::make_tuple(cols, lines);
}
