#include "ansi.h"
#include <sstream>
using std::stringstream;

string Ansi::set_color(const int& fg_rgb, const int& bg_rgb, int depth)
{
    stringstream ss;
    if (depth == 8)
    {
    }
    else if (depth == 256)
    {
    }
    else // 24 bit rgb
    {
        ss << esc;
        int r = (fg_rgb >> 16) & 0xff;
        int g = (fg_rgb >>  8) & 0xff;
        int b = (fg_rgb >>  0) & 0xff;
        ss << "38;2;" << r << ";" << g << ";" << b << "m";

        ss << esc;
        r = (bg_rgb >> 16) & 0xff;
        g = (bg_rgb >>  8) & 0xff;
        b = (bg_rgb >>  0) & 0xff;
        ss << "48;2;" << r << ";" << g << ";" << b << "m";
    }
    return ss.str();
}

string Ansi::set_pos(int x, int y)
{
    stringstream ss;
    ss << esc << y << ";" << x << "f";
    return ss.str();
}
