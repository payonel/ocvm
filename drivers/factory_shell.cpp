#include "basic_term.h"
#include "ansi_escape.h"

Framer* Factory::create_framer(const string& framerTypeName)
{
    if (framerTypeName == "basic")
    {
        return new BasicTerm;
    }
    else if (framerTypeName == "ansi")
    {
        return new AnsiEscapeTerm;
    }

    return nullptr;
}
