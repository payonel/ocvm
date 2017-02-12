#include "curses_shell.h"
#include "frame.h"
#include "log.h"
#include <curses.h>

#include <iostream>
#include <string>
#include <sstream>
using std::cout;
using std::flush;
using std::make_tuple;

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
    // auto max = maxResolution();

    int col, row;
    getmaxyx(stdscr, row, col);
    lout << "main window max resolution: " << col << "," << row << endl;

    int availableLines = row;
    size_t fullsizers = 0;

    for (auto& pf : _frames)
    {
        if (!pf->scrolling())
        {
            auto dim = pf->getResolution();
            availableLines -= std::get<1>(dim);
        }
        else
        {
            fullsizers++;
        }
    }

    int heightToBeDivided = availableLines;
    if (fullsizers)
    {
        heightToBeDivided /= fullsizers;
    }

    int top = 0;
    for (auto& pf : _frames)
    {
        WINDOW* pwin = _states[pf].window;
        auto dim = pf->getResolution();
        int width = std::get<0>(dim);
        int height = std::get<1>(dim);

        int y, x;
        getyx(pwin, y, x);

        bool remake = false;

        int curCol, curRow;
        getmaxyx(pwin, curRow, curCol);

        if (pf->scrolling())
        {
            if (heightToBeDivided != curRow)
            {
                remake = true;
            }
            
            height = heightToBeDivided;
        }
        
        if (remake || !width)
        {
            if (!width)
            {
                width = col;//max width
            }
            pf->setResolution(width, height, true);
        }

        if (y != top)
        {
            remake = true;
        }

        if (remake)
        {
            delwin(pwin);
            pwin = newwin(height, width, top, 0);
            scrollok(pwin, pf->scrolling());
            wrefresh(pwin);
            refresh();
            _states[pf].window = pwin;
        }

        top += height;
    }
}

tuple<int, int> CursesShell::maxResolution() const
{
    int col, row;
    getmaxyx(stdscr, row, col);
    lout << "main window max resolution: " << col << "," << row << endl;

    int availableLines = row;
    size_t fullsizers = 0;

    for (auto& pf : _frames)
    {
        if (!pf->scrolling())
        {
            auto dim = pf->getResolution();
            availableLines -= std::get<1>(dim);
        }
        else
        {
            fullsizers++;
        }
    }

    int heightToBeDivided = availableLines;
    if (fullsizers)
    {
        heightToBeDivided /= fullsizers;
    }

    return make_tuple(col, heightToBeDivided);
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
}
