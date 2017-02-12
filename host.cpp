#include "host.h"
#include "framing/frame.h"
#include "utils.h"
#include "components/component.h"
#include "components/screen.h"
#include "components/gpu.h"
#include "components/eeprom.h"
#include "components/computer.h"
#include "components/filesystem.h"
#include "components/keyboard.h"

class ScreenFrame : public Frame, public Screen
{
public:
    ScreenFrame()
    {
    }

    bool onInitialize(Value& config) override
    {
        return Screen::onInitialize(config);
    }

    void move(int x, int y) override
    {
    }

    bool setResolution(int width, int height) override
    {
        // resolution is +2 greater in each dim
        return Frame::setResolution(width + 2, height + 2);
    }

    void mouse(int x, int y) override
    {
        auto dim = getResolution();
        int w = std::get<0>(dim);
        int h = std::get<1>(dim);
        if (x <= 1 || x >= w || y <= 1 || y >= h)
            return; // do nothing
        Frame::mouse(x - 1, y - 1);
    }
};

Host::Host(Framer* framer) :
    _framer(framer)
{
}

Host::~Host()
{
    close();
}

string Host::machinePath() const
{
    return "system/machine.lua";
}

Component* Host::create(const string& type)
{
    if (type == "screen")
    {
        return new ScreenFrame;
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
    if (!_framer)
        return;

    delete _framer;
    _framer = nullptr;
}
