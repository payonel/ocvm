#include <string>
#include "host.h"
#include "client.h"
#include "shell.h"
#include "luaenv.h"

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
    Shell shell(&client);

    // init lua environment
    LuaEnv lenv;
    client.load(&lenv);

    // run lua machine
    lenv.load(host.machinePath());

    while (lenv.run())
    {
        shell.update();
    }

    lenv.close();
    shell.close();
    client.close();
    host.close();

    return 0;
}
