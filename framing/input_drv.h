#pragma once

#include <string>
using std::string;

struct KeyEvent
{
    string text;
    unsigned int keysym;
    unsigned int keycode;
    bool bPressed; // false: released

    bool bShift; //0x1
    bool bControl; // 0x4
    bool bAlt; // 0x8
};

struct MouseEvent
{
};

namespace InputDriver
{
    void stop();
    bool start();
    bool pop(KeyEvent* pke);
    bool pop(MouseEvent* pme);
};
