#include "ansi_escape.h"
#include <iostream>
using namespace std;

void AnsiEscapeTerm::print(AnsiFrameState* pf, int x, int y, const string& text)
{
    cout << text;
}

void refresh()
{
    cout << flush;
}

bool AnsiEscapeTerm::update()
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
                // log to file
            }
            else
            {
                int x = std::get<0>(buffer);
                int y = std::get<1>(buffer);
                print(&state, x, y, text);
            }
        }
        refresh();
    }

    //int ch = getch();
    //lout << "getch: " << ch << "\n";

    //if (ch == 3) // ^c
    //{
    //    lout << "shell closing\n";
    //    return false;
    //}

    return true;
}

bool AnsiEscapeTerm::open()
{
    return true;
}

void AnsiEscapeTerm::close()
{
    _states.clear();
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
