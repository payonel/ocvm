#pragma once

#include "input_drv.h"

#include <unordered_map>
#include <string>
using std::unordered_map;
using std::string;

struct KeyEvent
{
    string text;
    unsigned int keysym;
    unsigned int keycode;
    bool bPressed; // false: released

    bool bShift; //0x1
    bool bControl; // 0x4
    bool bAlt; // 0x8
};

class KeyboardDriver : public InputDriver<KeyEvent>
{
public:
    KeyboardDriver();

protected:
    uint map_code(const uint& code);
    uint map_sym(const uint& sym, int sequence_length);
    // TODO enqueue should take just K_RAW stdin data
    // void enqueue(char* bytes, uint length);
    void enqueue(bool bPressed, const string& text, uint keysym, uint sequence_length, uint keycode, uint state);
private:
    unordered_map<uint, uint> _codes;
};

namespace Factory
{
    KeyboardDriver* create_kb(const string& kbTypeName);
};

