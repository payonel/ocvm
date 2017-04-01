#pragma once

#include <string>
#include <vector>
using std::string;

class Color;

namespace Ansi
{
    static const char ESC = 0x1B;
    static const string esc = string{ESC} + "[";
    static const string cursor_on  = esc + "?25h";
    static const string cursor_off = esc + "?25l";
    static const string track_on   = esc + "?1002h";
    static const string track_off  = esc + "?1002l";
    static const string mouse_p_on   = esc + "?9h";
    static const string mouse_p_off  = esc + "?9l"; // press events only (X10)
    static const string mouse_pr_on   = esc + "?1000h";
    static const string mouse_pr_off  = esc + "?1000l"; // press and release events (X11)
    static const string mouse_prd_on   = esc + "?1003h";
    static const string mouse_prd_off  = esc + "?1003l"; // press, release, and drag events (X11)
    static const string clear_scroll = esc + "3J";
    static const string clear_term = esc + "2J" + clear_scroll;
    static const string clean_line = esc + "K";
    static const string save_pos   = esc + "s";
    static const string restore_pos= esc + "u";
    static const string color_reset= esc + "0m";

    string set_color(const Color& fg, const Color& bg);
    string set_pos(int x, int y);
};

