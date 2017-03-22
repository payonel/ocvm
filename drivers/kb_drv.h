#pragma once

#include "io/kb_input.h"
#include "term_buffer.h"

#include <tuple>
#include <unordered_map>
#include <string>
using std::tuple;
using std::unordered_map;
using std::string;

typedef unsigned int _Mod;
typedef unsigned char _Code;
typedef unsigned char _Sym;

class KeyboardDriverImpl : public KeyboardDriver
{
public:
    KeyboardDriverImpl();

    void enqueue(bool bPressed, _Code keycode);
    void enqueue(TermBuffer* buffer);

protected:
    void update_modifier(bool bPressed, _Code keycode);

private:
    unsigned char _mod_groups[8] {}; // 8 mod keys, 8 possible locations of those keys
    _Mod _modifier_state;
};
