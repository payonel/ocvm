#include <string>
#include "host.h"
#include "client.h"
#include "shell.h"
#include "luaenv.h"
#include "log.h"

int main(int argc, char** argv)
{
    std::string client_env_path = argc > 1 ? argv[1] : "tmp";

    // init host config
    // // prepares component factories such as screen, keyboard, and filesystem
    Host host(client_env_path);
    
    // init client config
    // // creates instances of host components
    Client client(&host);

    // create profile shell (houses screen component [list?])
    Shell shell;
    shell.add(&client);
    shell.add(&log);

    // init lua environment
    LuaEnv lenv;
    client.load(&lenv);

    // run lua machine
    if (!lenv.load(host.machinePath()))
    {
        return 1;
    }

    while (lenv.run())
    {
        shell.update();
    }

    return 0;
}
