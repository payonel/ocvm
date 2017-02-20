#include "curses_shell.h"
#include "frame.h"
#include "log.h"
#include <curses.h>

#include <iostream>
#include <string>
#include <sstream>
#include <list>
using std::list;
using std::cout;
using std::flush;
using std::make_tuple;

#include <chrono>
#include <thread>

#include <stdio.h>
#include <unistd.h>
#include <termios.h>

list<string> _rolling_buffer;
void push_log_history(const string& text)
{
    static const int LOG_DUMP_SIZE = 10000;
    _rolling_buffer.push_back(text);
    if (_rolling_buffer.size() > LOG_DUMP_SIZE)
    {
        _rolling_buffer.remove(_rolling_buffer.front());
    }
}

void dump_log_history()
{
    for (auto part : _rolling_buffer)
    {
        cout << part;
    }
    _rolling_buffer.clear();
}

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
    auto max = maxResolution();
    int maxWidth = std::get<0>(max);
    int maxHeight = std::get<1>(max);

    int top = 0;
    for (auto& pf : _frames)
    {
        WINDOW* pwin = _states[pf].window;
        auto dim = pf->getResolution();
        int width = std::get<0>(dim);
        int height = std::get<1>(dim);

        bool noDim = !width || !height;

        int y, x;
        getyx(pwin, y, x);

        int curWidth, curHeight;
        getmaxyx(pwin, curHeight, curWidth);

        bool remake = false;

        if (pf->scrolling())
        {
            if (curHeight != maxHeight)
            {
                remake = true;
            }
            
            height = maxHeight;
        }
        
        if (remake || noDim)
        {
            width = width ? width : maxWidth;
            height = height ? height : maxHeight;
            pf->setResolution(width, height, true);
        }

        bool badDim = curWidth != width || curHeight != height;
        remake = remake || badDim;

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
        auto dim = pf->getResolution();
        int height = std::get<1>(dim);
        // might be a fixed window
        // but if it has no height yet, it is acting full size
        if (height && !pf->scrolling())
        {
            availableLines -= height;
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
    //setlocale(LC_CTYPE, "");
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
    for (const auto& pf : _frames)
    {
        auto& state = _states[pf];
        while (!pf->empty())
        {
            auto buffer = pf->pop();
            string text = std::get<2>(buffer);

            if (pf->scrolling())
            {
                waddstr(state.window, text.c_str());
                push_log_history(text);
            }
            else
            {
                int x = std::get<0>(buffer);
                int y = std::get<1>(buffer);
                mvwprintw(state.window, y, x, text.c_str());
            }
        }
        wrefresh(state.window);
    }

    // wrefresh(stdscr);
    refresh();
    //int ch = getch();
    //lout << "getch: " << ch << "\n";

    //if (ch == 3) // ^c
    //{
    //    lout << "shell closing\n";
    //    return false;
    //}

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

    dump_log_history();
}
