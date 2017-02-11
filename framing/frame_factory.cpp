#include "frame_factory.h"

#include "curses_shell.h"

Framer* FrameFactory::create(EFramer eFramerType)
{
    switch (eFramerType)
    {
        case EFramer::Basic:
        break;
        case EFramer::Curses:
            return new CursesShell;
        break;
    }

    return nullptr;
}
