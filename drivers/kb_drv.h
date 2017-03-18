#pragma once

#include "io/kb_input.h"

#include <tuple>
#include <unordered_map>
#include <string>
using std::tuple;
using std::unordered_map;
using std::string;

class KeyboardDriverImpl : public KeyboardDriver
{
public:
    typedef uint ModifierMask;
    typedef unsigned char KeyCode;
    typedef unsigned char KeySym;

    KeyboardDriverImpl();

    void enqueue(bool bPressed, KeyCode keycode);
    void enqueue(unsigned char* keysequence, uint len);

protected:
    void update_modifier(bool bPressed, KeyCode keycode);
    KeySym map_sym(KeyCode code);

private:
    unsigned char _mod_groups[8] {}; // 8 mod keys, 8 possible locations of those keys
    ModifierMask _modifier_state;
};
