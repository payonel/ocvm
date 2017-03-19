#include "kb_drv.h"
#include "ansi.h"

#include <bitset>
#include <iostream>
using namespace std;

struct KBData
{
    unordered_map<KeyboardDriverImpl::KeyCode, tuple<uint, uint>> modifiers;
    unordered_map<KeyboardDriverImpl::KeyCode, tuple<KeyboardDriverImpl::KeySym, KeyboardDriverImpl::ModifierMask, KeyboardDriverImpl::KeySym>> syms;
    unordered_map<KeyboardDriverImpl::KeySym, tuple<KeyboardDriverImpl::KeyCode, KeyboardDriverImpl::ModifierMask>> codes;
    
    KBData()
    {
        // hard coded for now
        // termios should be able to provide this data
        syms.clear();
        
        // syms[keycode] = make_tuple(keysym_a, modifier_mask, keysym_b)
        // keysym_a represents keycode if modifier_state is active for the modifier_mask
        // i.e. modifier_state & modifier_mask is non-zero (true)
        // else, keysym_b
        syms[41] = make_tuple(126, 0xaaaa, 96);    // ` -30
        syms[1] = make_tuple(27, 0xffff, 0);       // escape
        syms[2] = make_tuple(33, 0xaaaa, 49);      // 1 16
        syms[3] = make_tuple(64, 0xaaaa, 50);      // 2 -14
        syms[4] = make_tuple(35, 0xaaaa, 51);      // 3 16
        syms[5] = make_tuple(36, 0xaaaa, 52);      // 4 16
        syms[6] = make_tuple(37, 0xaaaa, 53);      // 5 16
        syms[7] = make_tuple(94, 0xaaaa, 54);      // 6 -40
        syms[8] = make_tuple(38, 0xaaaa, 55);      // 7 17
        syms[9] = make_tuple(42, 0xaaaa, 56);      // 8 14
        syms[10] = make_tuple(40, 0xaaaa, 57);     // 9 17
        syms[11] = make_tuple(41, 0xaaaa, 48);     // 0 7
        syms[12] = make_tuple(95, 0xaaaa, 45);     // - -50
        syms[13] = make_tuple(43, 0xaaaa, 61);     // = 18
        syms[14] = make_tuple(8, 0xffff, 0);       // BACKSPACE
        syms[15] = make_tuple(9, 0x5555, 0);       // TAB
        syms[16] = make_tuple(81, 0x6666, 113);    // Q 32
        syms[17] = make_tuple(87, 0x6666, 119);    // W 32
        syms[18] = make_tuple(69, 0x6666, 101);    // E 32
        syms[19] = make_tuple(82, 0x6666, 114);    // R 32
        syms[20] = make_tuple(84, 0x6666, 116);    // T 32
        syms[21] = make_tuple(89, 0x6666, 121);    // Y 32
        syms[22] = make_tuple(85, 0x6666, 117);    // U 32
        syms[23] = make_tuple(73, 0x6666, 105);    // I 32
        syms[24] = make_tuple(79, 0x6666, 111);    // O 32
        syms[25] = make_tuple(80, 0x6666, 112);    // P 32
        syms[26] = make_tuple(123, 0xaaaa, 91);    // [ -32
        syms[27] = make_tuple(125, 0xaaaa, 93);    // ] -32
        syms[43] = make_tuple(124, 0xaaaa, 92);    // \ -32
        syms[30] = make_tuple(65, 0x6666, 97);     // A 32
        syms[31] = make_tuple(83, 0x6666, 115);    // S 32
        syms[32] = make_tuple(68, 0x6666, 100);    // D 32
        syms[33] = make_tuple(70, 0x6666, 102);    // F 32
        syms[34] = make_tuple(71, 0x6666, 103);    // G 32
        syms[35] = make_tuple(72, 0x6666, 104);    // H 32
        syms[36] = make_tuple(74, 0x6666, 106);    // J 32
        syms[37] = make_tuple(75, 0x6666, 107);    // K 32
        syms[38] = make_tuple(76, 0x6666, 108);    // L 32
        syms[39] = make_tuple(58, 0xaaaa, 59);     // ; 1
        syms[40] = make_tuple(34, 0xaaaa, 39);     // ' 5
        syms[28] = make_tuple(13, 0xffff, 0);      // ENTER
        syms[44] = make_tuple(90, 0x6666, 122);    // Z 32
        syms[45] = make_tuple(88, 0x6666, 120);    // X 32
        syms[46] = make_tuple(67, 0x6666, 99);     // C 32
        syms[47] = make_tuple(86, 0x6666, 118);    // V 32
        syms[48] = make_tuple(66, 0x6666, 98);     // B 32
        syms[49] = make_tuple(78, 0x6666, 110);    // N 32
        syms[50] = make_tuple(77, 0x6666, 109);    // M 32 ('m')
        syms[51] = make_tuple(60, 0xaaaa, 44);     // , -16
        syms[52] = make_tuple(62, 0xaaaa, 46);     // . -16
        syms[53] = make_tuple(63, 0xaaaa, 47);     // / -16
        syms[57] = make_tuple(32, 0xffff, 0);      // SPACE

        // modifiers[keycode] = make_tuple(modifier index, nth instance of that modifier)
        // shift:    0
        // caps:     1
        // control:  2
        // alt:      3
        // num lock: 4
        modifiers[ 42] = make_tuple(0, 0);  // left shift
        modifiers[ 54] = make_tuple(0, 1);  // right shift
        modifiers[ 58] = make_tuple(1, 0);  // caps lock
        modifiers[ 29] = make_tuple(2, 0);  // left control
        modifiers[157] = make_tuple(2, 1); // right control
        modifiers[ 56] = make_tuple(3, 0);  // left alt
        modifiers[184] = make_tuple(3, 1); // right alt
        modifiers[ 69] = make_tuple(4, 0);  // num lock

        codes.clear();
        for (auto s : syms)
        {
            auto key = s.first;
            auto stup = s.second;
            auto s0 = std::get<0>(stup);
            auto s1 = std::get<2>(stup);

            codes[s0] = make_tuple(key, 1);
            if (s1)
                codes[s1] = make_tuple(key, 0);
        }

        syms[71]  = make_tuple(55, 0xffff, 0);     // numpad 7
        syms[72]  = make_tuple(56, 0xffff, 0);     // numpad 8
        syms[73]  = make_tuple(57, 0xffff, 0);     // numpad 9
        syms[181] = make_tuple(47, 0xffff, 0);     // numpad /
        syms[75]  = make_tuple(52, 0xffff, 0);     // numpad 4
        syms[76]  = make_tuple(53, 0xffff, 0);     // numpad 5
        syms[77]  = make_tuple(54, 0xffff, 0);     // numpad 6
        syms[55]  = make_tuple(42, 0xffff, 0);     // numpad *
        syms[79]  = make_tuple(49, 0xffff, 0);     // numpad 1
        syms[80]  = make_tuple(50, 0xffff, 0);     // numpad 2
        syms[81]  = make_tuple(51, 0xffff, 0);     // numpad 3
        syms[74]  = make_tuple(45, 0xffff, 0);     // numpad -
        syms[82]  = make_tuple(48, 0xffff, 0);     // numpad 0
        syms[83]  = make_tuple(46, 0xffff, 0);     // numpad .
        syms[78]  = make_tuple(43, 0xffff, 0);     // numpad +
    }
} kb_data;

KeyboardDriverImpl::KeyboardDriverImpl()
{
    _modifier_state = 0;
}

struct SymJob
{
    unsigned char* buf;
    uint len;
    KeyboardDriverImpl::KeyCode code;
    KeyboardDriverImpl::ModifierMask mod;
    KeyboardDriverImpl::KeySym sym;

    bool computeSequenceContinuation(uint index)
    {
        if (index >= len)
            return false;

        switch (buf[index])
        {
            case 49: return computeHeader49(index + 1); // header start
            case 50: return computeHeader50(index + 1); // header start
            case 51: code = 211; return confirmEnd(index + 1, 126); // delete
            case 53: code = 201; return confirmEnd(index + 1, 126); // page up
            case 54: code = 209; return confirmEnd(index + 1, 126); // page down
            case 65: code = 200; return confirmEnd(index); // UP
            case 66: code = 208; return confirmEnd(index); // DOWN
            case 67: code = 205; return confirmEnd(index); // RIGHT
            case 68: code = 203; return confirmEnd(index); // LEFT
            case 70: code = 207; return confirmEnd(index); // END
            case 72: code = 199; return confirmEnd(index); // HOME
            case 90: code = 15; sym = 0; mod = 1; return confirmEnd(index); // shift tab
            default: return false;
        }

        return true;
    }

    bool confirmEnd(uint index, unsigned char end = 0)
    {
        if (index + 1 != len)
            return false;

        return !end || buf[index] == end;
    }

    bool computeSingal()
    {
        // not a sequence
        const auto& it = kb_data.codes.find(buf[0]);
        if (it != kb_data.codes.end())
        {
            code = std::get<0>(it->second);
            mod = std::get<1>(it->second);
            sym = buf[0];
            return true;
        }

        return false;
    }

    bool computeSequenceEnd(uint index)
    {
        if (index + 1 != len)
            return false;

        switch (buf[index])
        {
            case 80: code = 59; break; // F1
            case 81: code = 60; break; // F2
            case 82: code = 61; break; // F3
            case 83: code = 62; break; // F4
            default: return false;
        }

        return true;
    }

    bool computeMod(uint index)
    {
        if (index + 1 >= len)
            return false;

        switch (buf[index])
        {
            case 50: mod = 0x1; break; // shift
            case 51: mod = 0x4; break; // alt
            case 52: mod = 0x5; break; // shift + alt
            case 53: mod = 0x2; break; // control
            case 54: mod = 0x3; break; // shift+control
            default: return false;
        }

        return true;
    }

    bool computeHeader49(uint index)
    {
        if (index + 1 >= len)
            return false;

        switch (buf[index])
        {
            case 53: code = 63; return confirmEnd(index + 1, 126); // F5
            case 55: code = 64; return confirmEnd(index + 1, 126); // F6
            case 56: code = 65; return confirmEnd(index + 1, 126); // F7
            case 57: code = 66; return confirmEnd(index + 1, 126); // F8
            case 59: return computeMod(index + 1) && computeSequenceContinuation(index + 2); // compute mod
        }

        return false;
    }

    bool computeHeader50(uint index)
    {
        if (index + 1 >= len)
            return false;

        switch (buf[index])
        {
            case 48: code = 67; return confirmEnd(index + 1, 126); // F9
            case 49: code = 68; return confirmEnd(index + 1, 126); // F10
            case 51: code = 87; return confirmEnd(index + 1, 126); // F11
            case 52: code = 88; return confirmEnd(index + 1, 126); // F12
            case 57:
                code = 221;
                if (buf[index + 1] == 59)
                    return computeMod(index + 2) && confirmEnd(index + 3, 126);
                else
                    return confirmEnd(index + 1, 126);
        }

        return false;
    }

    bool computeEscapeSequence(uint index)
    {
        if (index + 1 >= len)
            return false;

        switch (buf[index])
        {
            case 79: return computeSequenceEnd(index + 1); break;
            case 91: return computeSequenceContinuation(index + 1); break;
        }

        return false;
    }

    bool computeSymJob()
    {
        if (len == 0) return false;

        if (len == 1)
        {
            return computeSingal();
        }
        else if (buf[0] == Ansi::ESC)
        {
            return computeEscapeSequence(1);
        }

        return false;
    }

    KeyboardDriverImpl::KeyCode modCode()
    {
        switch (mod)
        {
            case 0x01: return 42; // shift
            case 0x02: return 58; // caps
            case 0x04: return 29; // control
            case 0x08: return 56; // alt
            case 0x10: return 69; // num lock
        }

        return 0;
    }
};

void KeyboardDriverImpl::enqueue(unsigned char* buf, uint len)
{
    for (uint i = 0; i < len; i++)
    {
        if (buf[i] == 0)
        {
            len = i;
            break;
        }
    }

    cout << "keysym sequence: ";
    for (uint i = 0; i < len; i++)
        cout << (int)(buf[i]) << ' ';
    SymJob job { buf, len, 0, 0, 0 };
    if (!job.computeSymJob())
    {
        cout << " - unknown\r\n";
        return;
    }
    cout << "\r\n";

    update_modifier(true, job.modCode());
    enqueue(true, job.code);
}

void KeyboardDriverImpl::enqueue(bool bPressed, KeyboardDriverImpl::KeyCode keycode)
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
        case 221: // MENU
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

void KeyboardDriverImpl::update_modifier(bool bPressed, KeyboardDriverImpl::KeyCode keycode)
{
    const auto& modifier_set_iterator = kb_data.modifiers.find(keycode);
    if (modifier_set_iterator != kb_data.modifiers.end())
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

KeyboardDriverImpl::KeySym KeyboardDriverImpl::map_sym(KeyboardDriverImpl::KeyCode keycode)
{
    auto it = kb_data.syms.find(keycode);
    if (it == kb_data.syms.end())
        return 0x0;

    const auto& tup = it->second;
    return (std::get<1>(tup) & (0x1 << _modifier_state)) ? std::get<0>(tup) : std::get<2>(tup);
}

