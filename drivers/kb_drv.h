#pragma once

#include "io/kb_input.h"

#include <unordered_map>
#include <string>
using std::unordered_map;
using std::string;

class KeyboardDriverImpl : public KeyboardDriver
{
public:
    KeyboardDriverImpl();

    // TODO enqueue should take just K_RAW stdin data
    // void enqueue(char* bytes, uint length);
    void enqueue(bool bPressed, uint keysym, uint sequence_length, uint keycode, uint state);

protected:
    uint map_code(const uint& code);
    uint map_sym(const uint& sym, int sequence_length);

private:
    unordered_map<uint, uint> _codes;
    unordered_map<uint, uint> _syms;
};
