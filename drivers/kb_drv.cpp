#include "kb_drv.h"

#include <iostream>
using namespace std;

KeyboardDriverImpl::KeyboardDriverImpl()
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

    for (uint sym = 9; sym <= 61; sym++)
    {
        if (sym == 23 || sym == 37 || sym == 50)
            continue;
        _syms[sym] = 0xFFFF;
    }

    _syms[65] = 0xFFFF;
    _syms[94] = 0xFFFF;
    _syms[104] = 0xFFFF;
    _syms[109] = 0xFFFF;
    _syms[119] = 0xFFFF;
    _syms[125] = 0xFFFF;
    _syms[126] = 0xFFFF;
    _syms[129] = 0xFFFF;
    _syms[187] = 0xFFFF;
    _syms[188] = 0xFFFF;

    _syms[23] = 0x5555;

    for (uint code = 79; code <= 91; code++)
    {
        if (code == 82)
            continue;
        _syms[code] = 0xAAAA;
    }

    _syms[63] = 0xAFFF;
    _syms[82] = 0xAFFF;
    _syms[86] = 0xAFFF;
    _syms[106] = 0xAFFF;
}

void KeyboardDriverImpl::enqueue(bool bPressed, uint keycode, uint state)
{
    KeyEvent* pkey = new KeyEvent;
    pkey->bPressed = bPressed;
    pkey->keysym = map_sym(keycode, state);
    pkey->keycode = map_code(keycode);
    pkey->bShift = (state & 0x1);
    pkey->bControl = (state & 0x4);
    pkey->bAlt = (state & 0x8);

    _source->push(std::move(unique_ptr<KeyEvent>(pkey)));
}

uint KeyboardDriverImpl::map_code(uint code)
{
    const auto& it = _codes.find(code);
    if (it != _codes.end())
    {
        return it->second;
    }

    return code;
}

uint KeyboardDriverImpl::map_sym(uint keycode, uint state)
{
    auto it = _syms.find(keycode);
    if (it == _syms.end())
        return 0x0;

    uint state_mask = 0x1 << state;
    if (it->second & state_mask)
        return 0x1;

    return 0x0;
}

