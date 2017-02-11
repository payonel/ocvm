#include "frame_factory.h"

#include "basic_term.h"
#include "curses_shell.h"

Framer* FrameFactory::create(const string& framerTypeName)
{
    if (framerTypeName == "basic")
    {
        return new BasicTerm;
    }
    else if (framerTypeName == "curses")
    {
        return new CursesShell;
    }

    return nullptr;
}
