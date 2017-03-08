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
    KeyboardDriverImpl();

    // TODO enqueue should take just K_RAW stdin data
    // void enqueue(char* bytes, uint length);
    void enqueue(bool bPressed, uint keycode);

protected:
    void update_modifier(bool bPressed, uint keycode);
    uint map_code(uint code);
    uint map_sym(uint code);

private:
    unordered_map<uint, tuple<uint, uint>> _modifiers;
    unsigned char _mod_groups[8] {}; // 8 mod keys, 8 possible locations of those keys
    unordered_map<uint, uint> _codes;
    unordered_map<uint, uint> _syms;
    uint _modifier_state;
};
