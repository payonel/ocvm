#include "curses_shell.h"
#include "frame.h"
#include "log.h"
#include <ncurses.h>

#include <iostream>
#include <string>
#include <sstream>
using std::cout;
using std::flush;

#include <chrono>
#include <thread>

#include <stdio.h>
#include <unistd.h>
#include <termios.h>

CursesShell::CursesShell()
{
}

CursesShell::~CursesShell()
{
    close();
}

void CursesShell::onWrite(Frame* pWhichFrame)
{
}

void CursesShell::onResolution(Frame* pWhichFrame)
{
    int col, row;
    getmaxyx(stdscr, row, col);
    lout << "main window max resolution: " << col << "," << row << endl;

    auto dim = pWhichFrame->getResolution();
    int availableLines = row;
    size_t fullsizers = 0;

    for (auto& pf : _frames)
    {
        dim = pf->getResolution();
        int height = std::get<1>(dim);
        availableLines -= height;
        fullsizers += height == 0 ? 1 : 0;
    }

    int heightToBeDivided = 0;
    if (fullsizers)
    {
        heightToBeDivided = availableLines / fullsizers;
    }

    int top = 0;
    for (auto& pf : _frames)
    {
        WINDOW* pwin = _states[pf].window;
        dim = pf->getResolution();
        int height = std::get<1>(dim);

        int y, x;
        getyx(pwin, y, x);

        bool remake = false;

        int curCol, curRow;
        getmaxyx(pwin, curRow, curCol);

        if (!height)
        {
            if (heightToBeDivided != curRow)
            {
                remake = true;
            }
            
            height = heightToBeDivided;
        }

        if (y != top)
        {
            remake = true;
        }

        if (remake)
        {
            delwin(pwin);
            pwin = newwin(height, std::get<0>(dim), top, 0);
            scrollok(pwin, pf->scrolling());
            wrefresh(pwin);
            refresh();

            _states[pf].window = pwin;
        }

        top += height;
    }
}

Frame* CursesShell::getFrame(int x, int y) const
{
    for (auto pf : _frames)
    {
        const FrameState& state = _states.at(pf);

        int width, height;
        getmaxyx(state.window, height, width);
        int top, left;
        getbegyx(state.window, top, left);

        if (x >= left && x <= (left + width) && y >= top && y <= (top + height))
        {
            return pf;
        }
    }
    
    return nullptr;
}

bool CursesShell::onAdd(Frame* pf)
{
    WINDOW* pwin = newwin(0, 0, 0, 0);
    scrollok(pwin, pf->scrolling());
    _states[pf] = {pwin}; // full screen window
    onResolution(pf);

    return true;
}

bool CursesShell::open()
{
    initscr();

    // save current settings
    // put stdin in raw mode
    raw();
    keypad(stdscr, 1);
    noecho();
    curs_set(false);

    // switch to alternative buffer screen
    // enable mouse tracking
    // cout << clear();
    // cout << setPos(1, 1);
    // cout << flush;

    return true;
}

bool CursesShell::update()
{
    lout << "shell update\n";

    for (const auto& pf : _frames)
    {
        string buffer = pf->read();
        auto& state = _states[pf];

        if (pf->scrolling())
        {
            waddstr(state.window, buffer.c_str());
        }
        else
        {
            mvwprintw(state.window, pf->y(), pf->x(), buffer.c_str());
        }
        wrefresh(state.window);
    }

    refresh();
    int ch = 0;//getch();
    // lout << "getch: " << ch << "\n";

    if (ch == 3) // ^c
    {
        lout << "shell closing\n";
        return false;
    }

    return true;
}

void CursesShell::close()
{
    for (auto& pair : _states)
    {
        auto& state = pair.second;
        if (state.window)
        {
            delwin(state.window);
            state.window = nullptr;
        }
    }

    _states.clear();
    endwin();

    // disable mouse tracking
    // ::write(STDOUT_FILENO, "\e[?9l", 5);
    // screen buffer
    // ::write(STDOUT_FILENO, "\e[?47l", 6);
    // original attributes (we've had it in raw mode)
    // ::tcsetattr(STDIN_FILENO, TCSANOW, _original);
    // delete _original;
    // _original = nullptr;
}
