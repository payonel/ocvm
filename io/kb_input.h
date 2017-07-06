#pragma once

#include "input.h"
#include <vector>
using std::vector;

struct KeyEvent
{
    unsigned int keysym;
    unsigned int keycode;
    bool bPressed; // false: released

    bool bShift;    //0x01
    bool bCaps;     //0x02
    bool bControl;  //0x04
    bool bAlt;      //0x08
    bool bNumLock;  //0x10

    std::vector<char> insert;
};

class KeyboardInput : public InputSource<KeyEvent>
{
};
