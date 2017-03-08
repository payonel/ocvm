#include "kb_drv.h"

#include <bitset>
#include <iostream>
using namespace std;

KeyboardDriverImpl::KeyboardDriverImpl()
{
    _modifier_state = 0;

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

    // _modifiers[keycode] = make_tuple(modifier index, nth instance of that modifier)
    // shift: 1
    // control: 2
    // alt: 3
    _modifiers[50] = make_tuple(0, 0);
    _modifiers[62] = make_tuple(0, 1);
    _modifiers[66] = make_tuple(1, 0);
    _modifiers[37] = make_tuple(2, 0);
    _modifiers[105] = make_tuple(2, 1);
    _modifiers[64] = make_tuple(3, 0);
    _modifiers[108] = make_tuple(3, 1);
    _modifiers[205] = make_tuple(3, 2);
    _modifiers[77] = make_tuple(4, 0);
    _modifiers[133] = make_tuple(6, 0);
    _modifiers[134] = make_tuple(6, 1);
    _modifiers[206] = make_tuple(6, 2);
    _modifiers[207] = make_tuple(6, 3);
    _modifiers[92] = make_tuple(7, 0);
    _modifiers[203] = make_tuple(7, 1);
}

void KeyboardDriverImpl::enqueue(bool bPressed, uint keycode)
{
    update_modifier(bPressed, keycode);

    KeyEvent* pkey = new KeyEvent;
    pkey->bPressed = bPressed;
    pkey->keysym = map_sym(keycode);
    pkey->keycode = map_code(keycode);
    pkey->bShift = (_modifier_state & 0x1);
    pkey->bControl = (_modifier_state & 0x4);
    pkey->bAlt = (_modifier_state & 0x8);

    _source->push(std::move(unique_ptr<KeyEvent>(pkey)));
}

void KeyboardDriverImpl::update_modifier(bool bPressed, uint keycode)
{
    const auto& modifier_set_iterator = _modifiers.find(keycode);
    if (modifier_set_iterator != _modifiers.end())
    {
        const auto& mod_key_tuple = modifier_set_iterator->second;

        uint mod_index = std::get<0>(mod_key_tuple); // shift(0), lock(1), ctrl(2), etc
        uint nth_code = std::get<1>(mod_key_tuple); // the nth code in the group

        bitset<8> mod_bits = _mod_groups[mod_index];
        mod_bits.set(nth_code, bPressed);
        _mod_groups[mod_index] = static_cast<unsigned char>(mod_bits.to_ulong());

        bitset<8> state_bits = _modifier_state;
        state_bits.set(mod_index, mod_bits.any());
        _modifier_state = static_cast<unsigned char>(state_bits.to_ulong());
    }
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

uint KeyboardDriverImpl::map_sym(uint keycode)
{
    auto it = _syms.find(keycode);
    if (it == _syms.end())
        return 0x0;

    uint state_mask = 0x1 << _modifier_state;
    if (it->second & state_mask)
        return 0x1;

    return 0x0;
}

