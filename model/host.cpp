#include "model/host.h"
#include "io/frame.h"
#include "drivers/fs_utils.h"
#include "components/component.h"
#include "components/screen.h"
#include "components/gpu.h"
#include "components/eeprom.h"
#include "components/computer.h"
#include "components/filesystem.h"
#include "components/keyboard.h"

Host::Host(Framer* framer) :
    _framer(framer)
{
}

Host::~Host()
{
    close();
}

Component* Host::create(const string& type)
{
    if (type == "screen")
    {
        return new Screen;
    }
    else if (type == "gpu")
    {
        return new Gpu;
    }
    else if (type == "eeprom")
    {
        return new Eeprom;
    }
    else if (type == "computer")
    {
        return new Computer;
    }
    else if (type == "filesystem")
    {
        return new Filesystem;
    }
    else if (type == "keyboard")
    {
        return new Keyboard;
    }

    return nullptr;
}

Framer* Host::getFramer() const
{
    return _framer;
}

void Host::close()
{
}
