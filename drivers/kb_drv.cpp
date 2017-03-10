#include "kb_drv.h"

#include <bitset>
#include <iostream>
using namespace std;

KeyboardDriverImpl::KeyboardDriverImpl()
{
    _modifier_state = 0;

    // hard coded for now
    // termios should be able to provide this data
    _syms.clear();
    
    // syms[keycode] = make_tuple(keysym_a, modifier_mask, keysym_b)
    // keysym_a represents keycode if modifier_state is active for the modifier_mask
    // i.e. modifier_state & modifier_mask is non-zero (true)
    // else, keysym_b
    _syms[41] = make_tuple(126, 0xaaaa, 96);    // ` -30
    _syms[1] = make_tuple(27, 0xffff, 0);       // escape
    _syms[2] = make_tuple(33, 0xaaaa, 49);      // 1 16
    _syms[3] = make_tuple(64, 0xaaaa, 50);      // 2 -14
    _syms[4] = make_tuple(35, 0xaaaa, 51);      // 3 16
    _syms[5] = make_tuple(36, 0xaaaa, 52);      // 4 16
    _syms[6] = make_tuple(37, 0xaaaa, 53);      // 5 16
    _syms[7] = make_tuple(94, 0xaaaa, 54);      // 6 -40
    _syms[8] = make_tuple(38, 0xaaaa, 55);      // 7 17
    _syms[9] = make_tuple(42, 0xaaaa, 56);      // 8 14
    _syms[10] = make_tuple(40, 0xaaaa, 57);     // 9 17
    _syms[11] = make_tuple(51, 0xaaaa, 58);     // 10 7
    _syms[12] = make_tuple(95, 0xaaaa, 45);     // - -50
    _syms[13] = make_tuple(43, 0xaaaa, 61);     // = 18
    _syms[14] = make_tuple(8, 0xffff, 0);       // BACKSPACE
    _syms[15] = make_tuple(9, 0x5555, 0);       // TAB
    _syms[16] = make_tuple(81, 0x6666, 113);    // Q 32
    _syms[17] = make_tuple(87, 0x6666, 119);    // W 32
    _syms[18] = make_tuple(69, 0x6666, 101);    // E 32
    _syms[19] = make_tuple(82, 0x6666, 114);    // R 32
    _syms[20] = make_tuple(84, 0x6666, 116);    // T 32
    _syms[21] = make_tuple(89, 0x6666, 121);    // Y 32
    _syms[22] = make_tuple(85, 0x6666, 117);    // U 32
    _syms[23] = make_tuple(73, 0x6666, 105);    // I 32
    _syms[24] = make_tuple(79, 0x6666, 111);    // O 32
    _syms[25] = make_tuple(80, 0x6666, 112);    // P 32
    _syms[26] = make_tuple(123, 0xaaaa, 91);    // [ -32
    _syms[27] = make_tuple(125, 0xaaaa, 93);    // ] -32
    _syms[43] = make_tuple(124, 0xaaaa, 92);    // \ -32
    _syms[30] = make_tuple(65, 0x6666, 97);     // A 32
    _syms[31] = make_tuple(83, 0x6666, 115);    // S 32
    _syms[32] = make_tuple(68, 0x6666, 100);    // D 32
    _syms[33] = make_tuple(70, 0x6666, 102);    // F 32
    _syms[34] = make_tuple(71, 0x6666, 103);    // G 32
    _syms[35] = make_tuple(72, 0x6666, 104);    // H 32
    _syms[36] = make_tuple(74, 0x6666, 106);    // J 32
    _syms[37] = make_tuple(75, 0x6666, 107);    // K 32
    _syms[38] = make_tuple(76, 0x6666, 108);    // L 32
    _syms[39] = make_tuple(58, 0xaaaa, 59);     // ; 1
    _syms[40] = make_tuple(34, 0xaaaa, 39);     // ' 5
    _syms[28] = make_tuple(13, 0xffff, 0);      // ENTER
    _syms[44] = make_tuple(90, 0x6666, 122);    // Z 32
    _syms[45] = make_tuple(88, 0x6666, 120);    // X 32
    _syms[46] = make_tuple(67, 0x6666, 99);     // C 32
    _syms[47] = make_tuple(86, 0x6666, 118);    // V 32
    _syms[48] = make_tuple(66, 0x6666, 98);     // B 32
    _syms[49] = make_tuple(78, 0x6666, 110);    // N 32
    _syms[50] = make_tuple(77, 0x6666, 109);    // M 32 ('m')
    _syms[51] = make_tuple(60, 0xaaaa, 44);     // , -16
    _syms[52] = make_tuple(62, 0xaaaa, 46);     // . -16
    _syms[53] = make_tuple(63, 0xaaaa, 47);     // / -16
    _syms[57] = make_tuple(32, 0xffff, 0);      // SPACE

    _syms[71]  = make_tuple(55, 0xffff, 0);     // numpad 7
    _syms[72]  = make_tuple(56, 0xffff, 0);     // numpad 8
    _syms[73]  = make_tuple(57, 0xffff, 0);     // numpad 9
    _syms[181] = make_tuple(47, 0xffff, 0);     // numpad /
    _syms[75]  = make_tuple(52, 0xffff, 0);     // numpad 4
    _syms[76]  = make_tuple(53, 0xffff, 0);     // numpad 5
    _syms[77]  = make_tuple(54, 0xffff, 0);     // numpad 6
    _syms[55]  = make_tuple(42, 0xffff, 0);     // numpad *
    _syms[79]  = make_tuple(49, 0xffff, 0);     // numpad 1
    _syms[80]  = make_tuple(50, 0xffff, 0);     // numpad 2
    _syms[81]  = make_tuple(51, 0xffff, 0);     // numpad 3
    _syms[74]  = make_tuple(45, 0xffff, 0);     // numpad -
    _syms[82]  = make_tuple(48, 0xffff, 0);     // numpad 0
    _syms[83]  = make_tuple(46, 0xffff, 0);     // numpad .
    _syms[78]  = make_tuple(43, 0xffff, 0);     // numpad +

    // _modifiers[keycode] = make_tuple(modifier index, nth instance of that modifier)
    // shift:    0
    // caps:     1
    // control:  2
    // alt:      3
    // num lock: 4
    _modifiers[42] = make_tuple(0, 0);  // left shift
    _modifiers[54] = make_tuple(0, 1);  // right shift
    _modifiers[58] = make_tuple(1, 0);  // caps lock
    _modifiers[29] = make_tuple(2, 0);  // left control
    _modifiers[157] = make_tuple(2, 1); // right control
    _modifiers[56] = make_tuple(3, 0);  // left alt
    _modifiers[184] = make_tuple(3, 1); // right alt
    _modifiers[69] = make_tuple(4, 0);  // num lock
}

void KeyboardDriverImpl::enqueue(bool bPressed, uint keycode)
{
    // filter out some events
    switch (keycode)
    {
        case 95|0x80: // FN+F3 (SLEEP) (double byte)
        case 86|0x80: // FN+F4 (DISPLAY) (double byte)
        case 46|0x80: // FN+F6 (SPEAKER VOLUME DOWN) (double byte)
        case 48|0x80: // FN+F7 (SPEAKER VOLUME UP) (double byte)
        case 76|0x80: // FN+F9 (DISPLAY BACKLIGHT DECREASE) (double byte)
        case 84|0x80: // FN+F10 (DISPLAY BACKLIGHT INCREASE) (double byte)
        case 42|0x80: // PRINT SCREEN (comes in a pair of double bytes, 42,55 -- each are pressed and unpressed)
        case 55|0x80: // PRINT SCREEN (comes in a pair of double bytes, 42,55 -- each are pressed and unpressed)
            return;
        case 91: // WINDOWS
        case 93: // MENU
            keycode = 0;
            break;
    }

    KeyEvent* pkey = new KeyEvent;
    pkey->bPressed = bPressed;
    pkey->keycode = keycode;

    update_modifier(bPressed, pkey->keycode);
    pkey->keysym = map_sym(pkey->keycode);

    pkey->bShift = (_modifier_state & 0x1);
    pkey->bCaps = (_modifier_state & 0x2);
    pkey->bControl = (_modifier_state & 0x4);
    pkey->bAlt = (_modifier_state & 0x8);
    pkey->bNumLock = (_modifier_state & 0x10);

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

uint KeyboardDriverImpl::map_sym(uint keycode)
{
    auto it = _syms.find(keycode);
    if (it == _syms.end())
        return 0x0;

    const auto& tup = it->second;
    return (std::get<1>(tup) & (0x1 << _modifier_state)) ? std::get<0>(tup) : std::get<2>(tup);
}

