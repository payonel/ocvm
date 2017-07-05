#pragma once

#include "io/kb_input.h"
#include "term_buffer.h"

#include <tuple>
#include <unordered_map>
#include <string>
#include <list>
using std::tuple;
using std::unordered_map;
using std::string;
using std::list;

typedef unsigned int _Mod;
typedef unsigned char _Code;
typedef unsigned char _Sym;

class KeyboardTerminalDriver
{
public:
    KeyboardDriverImpl();

    void enqueue(bool bPressed, _Code keycode);
    void enqueue(TermBuffer* buffer);
    unique_ptr<KeyboardEvent> parse(TermBuffer* buffer);

protected:
    void update_modifier(bool bPressed, _Code keycode);

private:
    unsigned char _mod_groups[8] {}; // 8 mod keys, 8 possible locations of those keys
    _Mod _modifier_state;

    unordered_map<_Code, list<_Code>::iterator> _pressedCodesCache;
    list<_Code> _lastUsedCodes;
    const size_t cache_size = 3;
};

class KeyboardLocalRawTtyDriver : public KeyboardTerminalDriver
{
public:
    unique_ptr<KeyboardEvent> parse(TermBuffer* buffer) override;
};

class KeyboardPtyDriver : public KeyboardTerminalDriver
{
public:
    unique_ptr<KeyboardEvent> parse(TermBuffer* buffer) override;
};

