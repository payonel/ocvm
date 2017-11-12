#include "model/host.h"
#include "drivers/fs_utils.h"
#include "components/component.h"
#include "components/screen.h"
#include "components/gpu.h"
#include "components/eeprom.h"
#include "components/computer.h"
#include "components/filesystem.h"
#include "components/keyboard.h"
#include "components/internet.h"
#include "components/sandbox.h"
#include "components/modem.h"
#include "apis/unicode.h"

Host::Host(string frameType) :
    _frameType(frameType)
{
}

Host::~Host()
{
    close();
}

Component* Host::create(const string& type) const
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
    else if (type == "internet")
    {
        return new Internet;
    }
    else if (type == "sandbox")
    {
        return new Sandbox;
    }
    else if (type == "modem")
    {
        return new Modem;
    }

    return nullptr;
}

Frame* Host::createFrame() const
{
    return Factory::create_frame(_frameType);
}

void Host::close()
{
}

string Host::stackLog() const
{ 
    return _stack_log;
}

void Host::stackLog(const string& stack_log)
{
    _stack_log = stack_log;
}

string Host::biosPath() const
{
    return _bios_path;
}

void Host::biosPath(const string& bios_path)
{
    _bios_path = bios_path;
}

string Host::fontsPath() const
{
    return _fonts_path;
}

void Host::fontsPath(const string& fonts_path)
{
    _fonts_path = fonts_path;
    if (!UnicodeApi::configure(_fonts_path))
    {
        std::cerr << "Failed lot load fonts: " << fonts_path << std::endl;
        ::exit(1);
    }
}

string Host::machinePath() const
{
    return _machine_path;
}

void Host::machinePath(const string& machine_path)
{
    _machine_path = machine_path;
}
