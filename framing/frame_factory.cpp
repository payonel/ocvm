#include "frame_factory.h"

#include "basic_term.h"
#include "curses_shell.h"

Framer* FrameFactory::create(EFramer eFramerType)
{
    switch (eFramerType)
    {
        case EFramer::Basic:
            return new BasicTerm;
        break;
        case EFramer::Curses:
            return new CursesShell;
        break;
    }

    return nullptr;
}
