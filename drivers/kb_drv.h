#pragma once

#include "io/event.h"

#include <vector>
using std::vector;

class TermBuffer;

class KeyboardTerminalDriver
{
public:
    virtual vector<KeyEvent> parse(TermBuffer* buffer) = 0;
    virtual vector<KeyEvent> idle() = 0;
    virtual ~KeyboardTerminalDriver() {}

    static std::unique_ptr<KeyboardTerminalDriver> create(bool bMaster);
};

