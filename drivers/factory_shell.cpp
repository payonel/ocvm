#include "basic_term.h"
#include "ansi_escape.h"

Frame* Factory::create_frame(const string& frameTypeName)
{
    if (frameTypeName == "basic")
    {
        return new BasicTerm;
    }
    else if (frameTypeName == "ansi")
    {
        return new AnsiEscapeTerm;
    }

    return nullptr;
}
