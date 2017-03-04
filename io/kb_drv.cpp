#include "kb_drv.h"

#include <iostream>
using namespace std;

KeyboardDriver::KeyboardDriver()
{
    // [9, 96] = x-8
    for (uint src = 9; src <= 96; src++)
    {
        _codes[src] = src - 8;
    }
    // [133, 137] = x+86
    for (uint src = 133; src <= 137; src++)
    {
        _codes[src] = src + 86;
    }

    _codes[105] = 157;
    _codes[108] = 184;

    _codes[127] = 197;

    _codes[110] = 199;
    _codes[111] = 200;
    _codes[112] = 201;
    _codes[113] = 203;
    _codes[114] = 205;
    _codes[115] = 207;
    _codes[116] = 208;
    _codes[117] = 209;
    _codes[118] = 210;
    _codes[119] = 211;
}

void KeyboardDriver::enqueue(bool bPressed, const string& text, uint keysym, uint sequence_length, uint keycode, uint state)
{
    KeyEvent* pkey = new KeyEvent;
    pkey->bPressed = bPressed;
    pkey->text = text;
    pkey->keysym = map_sym(keysym, sequence_length);
    pkey->keycode = map_code(keycode);
    pkey->bShift = (state & 0x1);
    pkey->bControl = (state & 0x4);
    pkey->bAlt = (state & 0x8);

    push(std::move(unique_ptr<InputEvent>(pkey)));
}

uint KeyboardDriver::map_code(const uint& code)
{
    const auto& it = _codes.find(code);
    if (it != _codes.end())
    {
        return it->second;
    }

    return code;
}

uint KeyboardDriver::map_sym(const uint& keysym, int sequence_length)
{
    if (sequence_length == 0)
        return 0;

    if (keysym & 0xFF00)
        return keysym & 0x7F;

    return keysym;
}

