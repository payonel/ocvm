#pragma once

#include "input.h"
#include <string>
using std::string;

class KeyboardDriver : public InputDriver
{
};

struct KeyEvent : public InputEvent
{
    string text;
    unsigned int keysym;
    unsigned int keycode;
    bool bPressed; // false: released

    bool bShift; //0x1
    bool bControl; // 0x4
    bool bAlt; // 0x8
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
    unique_ptr<KeyboardDriver> create_kb(const string& kbTypeName);
};
