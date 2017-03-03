#pragma once

#include <string>
using std::string;

namespace Ansi
{
    static const char ESC = 0x1B;
    static const string esc = string{ESC} + "[";
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
    static const string color_reset= esc + "0m";

    string set_color(const int& fg_rgb, const int& bg_rgb, int depth);
    string set_pos(int x, int y);
};

