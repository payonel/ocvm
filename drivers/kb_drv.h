#pragma once

#include "io/event.h"
#include "term_buffer.h"

#include <tuple>
#include <unordered_map>
#include <string>
#include <list>
#include <vector>
using std::tuple;
using std::unordered_map;
using std::string;
using std::list;
using std::vector;

typedef unsigned int _Mod;
typedef unsigned char _Code;
typedef unsigned char _Sym;

class KeyboardTerminalDriver
{
public:
    KeyboardTerminalDriver();

    void mark(bool bPressed, _Code keycode, vector<KeyEvent>* pOut);
    virtual vector<KeyEvent> parse(TermBuffer* buffer) = 0;

protected:
    void update_modifier(bool bPressed, _Code keycode);
    _Mod _modifier_state;

private:
    unsigned char _mod_groups[8] {}; // 8 mod keys, 8 possible locations of those keys
};

class KeyboardLocalRawTtyDriver : public KeyboardTerminalDriver
{
public:
    vector<KeyEvent> parse(TermBuffer* buffer) override;
};

class KeyboardPtyDriver : public KeyboardTerminalDriver
{
public:
    vector<KeyEvent> parse(TermBuffer* buffer) override;
private:
    list<_Code> _lastUsedCodes;
    unordered_map<_Code, list<_Code>::iterator> _pressedCodesCache;
    const size_t cache_size = 3;
};

