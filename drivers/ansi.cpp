#include "ansi.h"
#include "color/color_types.h"
#include <algorithm>
#include <map>
#include <sstream>
#include <vector>
using std::stringstream;

#include <iostream>

// clang-format off
static const unsigned char oc_to_ansi[256] =
{
    234,235,236,238,239,240,241,242,244,245,246,247,249,250,251,253, // grays
     16, 17, 18, 19, 21,  16, 17, 18, 19, 21,  22, 23, 24, 25, 27,  22, 23, 24, 25, 27,
     28, 29, 30, 31, 33,  34, 35, 36, 37, 39,  40, 41, 42, 43, 45,  46, 47, 48, 49, 51,

     52, 53, 54, 55, 57,  52, 53, 54, 55, 57,  58, 59, 60, 61, 63,  58, 59, 60, 61, 63,
     64, 65, 66, 67, 69,  70, 71, 72, 73, 75,  76, 77, 78, 79, 81,  82, 83, 84, 85, 87,

     52, 53, 54, 55, 57,  52, 53, 54, 55, 57,  58, 59, 60, 61, 63,  58, 59, 60, 61, 63,
     64, 65, 66, 67, 69,  70, 71, 72, 73, 75,  76, 77, 78, 79, 81,  82, 83, 84, 85, 87,

     88, 89, 90, 91, 93,  88, 89, 90, 91, 93,  94, 95, 96, 97, 99,  94, 95, 96, 97, 99,
    100,101,102,103,105, 106,107,108,109,111, 112,113,114,115,117, 118,119,120,121,123,

    160,161,162,163,165, 160,161,162,163,165, 166,167,168,169,171, 166,167,168,169,171,
    172,173,174,175,177, 178,179,180,181,183, 184,185,186,187,189, 190,191,192,193,195,

      1,197,198,199,201, 196,197,198,199,201, 202,203,204,205,207, 202,203,204,205,207,
    208,209,210,211,213, 214,215,216,217,219, 220,221,222,223,225, 226,227,228,229,255,
};
// clang-format on

string to_ansi(unsigned char deflated_rgb, bool foreground)
{
  stringstream ss;
  ss << (foreground ? "3" : "4") << "8;5;" << (int)oc_to_ansi[deflated_rgb];
  return ss.str();
}

string Ansi::set_color(const Color& fg, const Color& bg)
{
  string fg_txt = to_ansi(fg.code, true);
  string bg_txt = to_ansi(bg.code, false);

  return esc + fg_txt + ";" + bg_txt + "m";
}

string Ansi::set_pos(int x, int y)
{
  stringstream ss;
  ss << esc << y << ";" << x << "f";
  return ss.str();
}
