#include "host.h"
#include "component.h"

#include "screen.h"
#include "config.h"
#include <sys/stat.h>

using std::string;

Host::Host(const string& env_path) : _env_path(env_path)
{
    // make the env path if it doesn't already exist
    mkdir(env_path.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
}

Host::~Host()
{
}

string Host::machinePath() const
{
    return "system/machine.lua";
}

string Host::envPath() const
{
    return _env_path;
}

Component* Host::create(const string& type)
{
    if (type == "screen")
    {
        return new Screen;
    }

    return nullptr;
}

void Host::close()
{
}
