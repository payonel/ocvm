#include "basic_term.h"
#include "curses_shell.h"
#include "ansi_escape.h"

#include "kb_scanner.h"

#include "mouse_raw.h"

Framer* Factory::create_framer(const string& framerTypeName)
{
    if (framerTypeName == "basic")
    {
        return new BasicTerm;
    }
    else if (framerTypeName == "curses")
    {
        return new CursesShell;
    }
    else if (framerTypeName == "ansi")
    {
        return new AnsiEscapeTerm;
    }

    return nullptr;
}

KeyboardDriver* Factory::create_kb(const string& kbTypeName)
{
    if (kbTypeName == "scanner")
    {
        return new KeyboardScanner;
    }

    return nullptr;
}

MouseDriver* Factory::create_mouse(const string& mouseTypeName)
{
    return nullptr;
}
