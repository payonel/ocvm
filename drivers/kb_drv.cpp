#include "kb_drv.h"

#include <bitset>
#include <iostream>
using namespace std;

KeyboardDriverImpl::KeyboardDriverImpl()
{
    _modifier_state = 0;

    // _codes maps raw kb keycodes to keycode expected by oc

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

    // hard coded for now
    // termios should be able to provide this data
    _syms.clear();
    
    // syms[keycode] = make_tuple(keysym_a, modifier_mask, keysym_b)
    // keysym_a represents keycode if modifier_state is active for the modifier_mask
    // i.e. modifier_state & modifier_mask is non-zero (true)
    // else, keysym_b
    _syms[9] = make_tuple(27, 0xffff, 0);
    _syms[10] = make_tuple(33, 0xaaaa, 49);// 16
    _syms[11] = make_tuple(64, 0xaaaa, 50);// -14
    _syms[12] = make_tuple(35, 0xaaaa, 51);// 16
    _syms[13] = make_tuple(36, 0xaaaa, 52);// 16
    _syms[14] = make_tuple(37, 0xaaaa, 53);// 16
    _syms[15] = make_tuple(94, 0xaaaa, 54);// -40
    _syms[16] = make_tuple(38, 0xaaaa, 55);// 17
    _syms[17] = make_tuple(42, 0xaaaa, 56);// 14
    _syms[18] = make_tuple(40, 0xaaaa, 57);// 17
    _syms[19] = make_tuple(41, 0xaaaa, 48);// 7
    _syms[20] = make_tuple(95, 0xaaaa, 45);// -50
    _syms[21] = make_tuple(43, 0xaaaa, 61);// 18
    _syms[22] = make_tuple(8, 0xffff, 0);
    _syms[23] = make_tuple(9, 0x5555, 0);
    _syms[24] = make_tuple(81, 0x6666, 113);// 32
    _syms[25] = make_tuple(87, 0x6666, 119);// 32
    _syms[26] = make_tuple(69, 0x6666, 101);// 32
    _syms[27] = make_tuple(82, 0x6666, 114);// 32
    _syms[28] = make_tuple(84, 0x6666, 116);// 32
    _syms[29] = make_tuple(89, 0x6666, 121);// 32
    _syms[30] = make_tuple(85, 0x6666, 117);// 32
    _syms[31] = make_tuple(73, 0x6666, 105);// 32
    _syms[32] = make_tuple(79, 0x6666, 111);// 32
    _syms[33] = make_tuple(80, 0x6666, 112);// 32
    _syms[34] = make_tuple(123, 0xaaaa, 91);// -32
    _syms[35] = make_tuple(125, 0xaaaa, 93);// -32
    _syms[36] = make_tuple(13, 0xffff, 0);
    _syms[38] = make_tuple(65, 0x6666, 97);// 32
    _syms[39] = make_tuple(83, 0x6666, 115);// 32
    _syms[40] = make_tuple(68, 0x6666, 100);// 32
    _syms[41] = make_tuple(70, 0x6666, 102);// 32
    _syms[42] = make_tuple(71, 0x6666, 103);// 32
    _syms[43] = make_tuple(72, 0x6666, 104);// 32
    _syms[44] = make_tuple(74, 0x6666, 106);// 32
    _syms[45] = make_tuple(75, 0x6666, 107);// 32
    _syms[46] = make_tuple(76, 0x6666, 108);// 32
    _syms[47] = make_tuple(58, 0xaaaa, 59);// 1
    _syms[48] = make_tuple(34, 0xaaaa, 39);// 5
    _syms[49] = make_tuple(126, 0xaaaa, 96);// -30
    _syms[51] = make_tuple(124, 0xaaaa, 92);// -32
    _syms[52] = make_tuple(90, 0x6666, 122);// 32
    _syms[53] = make_tuple(88, 0x6666, 120);// 32
    _syms[54] = make_tuple(67, 0x6666, 99);// 32
    _syms[55] = make_tuple(86, 0x6666, 118);// 32
    _syms[56] = make_tuple(66, 0x6666, 98);// 32
    _syms[57] = make_tuple(78, 0x6666, 110);// 32
    _syms[58] = make_tuple(77, 0x6666, 109);// 32
    _syms[59] = make_tuple(60, 0xaaaa, 44);// -16
    _syms[60] = make_tuple(62, 0xaaaa, 46);// -16
    _syms[61] = make_tuple(63, 0xaaaa, 47);// -16
    _syms[63] = make_tuple(42, 0xafff, 0);
    _syms[65] = make_tuple(32, 0xffff, 0);
    _syms[79] = make_tuple(55, 0xaaaa, 0);
    _syms[80] = make_tuple(56, 0xaaaa, 0);
    _syms[81] = make_tuple(57, 0xaaaa, 0);
    _syms[82] = make_tuple(45, 0xafff, 0);
    _syms[83] = make_tuple(52, 0xaaaa, 0);
    _syms[84] = make_tuple(53, 0xaaaa, 0);
    _syms[85] = make_tuple(54, 0xaaaa, 0);
    _syms[86] = make_tuple(43, 0xafff, 0);
    _syms[87] = make_tuple(49, 0xaaaa, 0);
    _syms[88] = make_tuple(50, 0xaaaa, 0);
    _syms[89] = make_tuple(51, 0xaaaa, 0);
    _syms[90] = make_tuple(48, 0xaaaa, 0);
    _syms[91] = make_tuple(46, 0xaaaa, 0);
    _syms[94] = make_tuple(62, 0xaaaa, 60);// -2
    _syms[104] = make_tuple(13, 0xffff, 0);
    _syms[106] = make_tuple(47, 0xafff, 0);
    _syms[109] = make_tuple(10, 0xffff, 0);
    _syms[119] = make_tuple(127, 0xffff, 0);
    _syms[125] = make_tuple(61, 0xffff, 0);
    _syms[126] = make_tuple(177, 0xffff, 0);
    _syms[129] = make_tuple(46, 0xffff, 0);
    _syms[187] = make_tuple(40, 0xffff, 0);
    _syms[188] = make_tuple(41, 0xffff, 0);

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

    const auto& tup = it->second;
    return (std::get<1>(tup) & (0x1 << _modifier_state)) ? std::get<0>(tup) : std::get<2>(tup);
}

