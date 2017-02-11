#include <string>
#include "host.h"
#include "client.h"
#include "curses_shell.h"
#include "luaenv.h"
#include "log.h"

using std::cerr;

int main(int argc, char** argv)
{
    string client_env_path = argc > 1 ? argv[1] : "tmp";
    // init host config
    // // prepares component factories such as screen, keyboard, and filesystem
    Host host(client_env_path);

    // create profile shell (houses screen component [list?])
    Framer* framer = host.getFramer();
    if (!framer)
    {
        cerr << "no framer available\n";
        return 1;
    }

    if (!framer->open())  // open the ui
    {
        cerr << "framer open failed\n";
        return 1;
    }

    framer->add(Logger::getFrame());

    // init client config
    // // creates instances of host components
    Client client(&host);

    // init lua environment
    LuaEnv lenv;
    if (!client.load(&lenv))
    {
        return 1;
    }

    // run lua machine
    if (!lenv.load(host.machinePath()))
    {
        return 1;
    }

    while (framer->update())
    {
        if (!lenv.run())
        {
            break;
        }
    }

    framer->close();

    return 0;
}
