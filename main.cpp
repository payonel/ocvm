#include <string>
#include <memory>
#include "host.h"
#include "client.h"
#include "framing/frame_factory.h"
#include "framing/frame.h"
#include "log.h"

using std::cerr;

int main(int argc, char** argv)
{
    string client_env_path = argc > 1 ? argv[1] : "tmp";
    string framer_type = argc > 2 ? argv[2] : "ansi";
    std::unique_ptr<Framer> framer(FrameFactory::create(framer_type));

    // create profile shell (houses screen component [list?])
    if (!framer)
    {
        cerr << "no [" << framer_type << "] framer available\n";
        return 1;
    }

    // init host config
    // // prepares component factories such as screen, keyboard, and filesystem
    Host host(framer.get());

    if (!framer->open())  // open the ui
    {
        cerr << "framer open failed\n";
        return 1;
    }

    framer->add(Logger::getFrame());

    // init client config
    // // creates instances of host components
    Client client(&host, client_env_path);

    // init lua environment
    if (!client.load())
    {
        return 1;
    }

    while (framer->update())
    {
        if (!client.run())
        {
            break;
        }
    }

    return 0;
}
