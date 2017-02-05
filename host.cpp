#include "host.h"
#include "shell.h"
#include "components/component.h"
#include "components/screen.h"
#include <sys/stat.h>

using std::string;

class ScreenFrame : public Frame, public Screen
{
public:
    ScreenFrame(const Value& init) : Screen(init)
    {
    }

    bool setResolution(int width, int height) override
    {
        // resolution is +2 greater in each dim
        // draw blanks if shrinking
        int w, h;
        getResolution(&w, &h);

        return Frame::setResolution(width + 2, height + 2);
    }

    void mouse(int x, int y) override
    {
        int w, h;
        getResolution(&w, &h);
        if (x <= 1 || x >= w || y <= 1 || y >= h)
            return; // do nothing
        Frame::mouse(x - 1, y - 1);
    }
};

Host::Host(const string& env_path) : _env_path(env_path)
{
    // make the env path if it doesn't already exist
    mkdir(env_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);

    _framer = new Shell;
}

Host::~Host()
{
    close();
}

string Host::machinePath() const
{
    return "system/foobar.lua";
    // return "system/machine.lua";
}

string Host::envPath() const
{
    return _env_path;
}

Component* Host::create(const string& type, const Value& value)
{
    if (type == "screen")
    {
        auto* p = new ScreenFrame(value);
        getFramer()->add(p);
        return p;
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
