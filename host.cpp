#include "host.h"
#include "shell.h"
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
    ScreenFrame(const string& type, const Value& init, Host* host) :
        Screen(type, init, host)
    {
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

Host::Host(const string& env_path) : _env_path(env_path)
{
    // make the env path if it doesn't already exist
    this->mkdir("");
    
    _framer = new Shell;
}

Host::~Host()
{
    close();
}

void Host::mkdir(const string& path)
{
    utils::mkdir(_env_path + "/" + path);
}

string Host::machinePath() const
{
    return "system/machine.lua";
}

string Host::envPath() const
{
    return _env_path;
}

Component* Host::create(const string& type, const Value& init)
{
    Component* p = nullptr;

    if (type == "screen")
    {
        auto* sf = new ScreenFrame(type, init, this);
        getFramer()->add(sf, 0); // insert at top
        //sf->setResolution(50, 10);
        p = sf;
    }
    else if (type == "gpu")
    {
        p = new Gpu(type, init, this);
    }
    else if (type == "eeprom")
    {
        p = new Eeprom(type, init, this);
    }
    else if (type == "computer")
    {
        p = new Computer(type, init, this);
    }
    else if (type == "filesystem")
    {
        p = new Filesystem(type, init, this);
    }
    else if (type == "keyboard")
    {
        p = new Keyboard(type, init, this);
    }

    return p;
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
