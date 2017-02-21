#include "keyboard.h"
#include "screen.h"
#include "client.h"
#include "log.h"

Keyboard::Keyboard()
{
}

bool Keyboard::onInitialize(Value& config)
{
    _preferredScreen = config.get(3).Or("").toString();
    return true;
}

bool Keyboard::postInit()
{
    for (auto* pc : client()->components("screen", true))
    {
        Screen* screen = dynamic_cast<Screen*>(pc);
        if (screen && (_preferredScreen.empty() || _preferredScreen == screen->address()))
        {
            screen->addKeyboard(address());
            return true;
        }
    }
    lout << "warning: kb had no screen to join\n";
    return true;
}