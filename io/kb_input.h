#pragma once

#include "input.h"
#include <string>
using std::string;

class KeyboardDriver : public InputDriver
{
};

struct KeyEvent : public InputEvent
{
    unsigned int keysym;
    unsigned int keycode;
    bool bPressed; // false: released

    bool bShift;    //0x01
    bool bCaps;     //0x02
    bool bControl;  //0x04
    bool bAlt;      //0x08
    bool bNumLock;  //0x10
};

class KeyboardInput : protected InputSource
{
public:
    bool open(unique_ptr<KeyboardDriver> driver);
    using InputSource::close;

    unique_ptr<KeyEvent> pop();
private:
};

namespace Factory
{
    unique_ptr<KeyboardDriver> create_kb(const string& kbTypeName = "");
};
