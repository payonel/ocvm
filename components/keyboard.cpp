#include "keyboard.h"
#include "screen.h"
#include "client.h"
#include "log.h"

#include "drivers/kb_drv.h"
#include "drivers/kb_scanner.h"

Keyboard::Keyboard()
{
}

Keyboard::~Keyboard()
{
    if (_driver)
        _driver->stop();

    delete _driver;
    _driver = nullptr;
}

bool Keyboard::onInitialize(Value& config)
{
    _preferredScreen = config.get(3).Or("").toString();

    _driver = new KeyboardScanner;
    _driver->start();

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

RunState Keyboard::update()
{
    KeyEvent ke;
    if (_driver->pop(&ke))
    {
        if (ke.keycode == 1)
        {
            lout << "shell abort";
            return RunState::Halt;
        }
        else
        {
            client()->pushSignal({ke.bPressed ? "key_down" : "key_up", address(), ke.keysym, ke.keycode});
        }
    }

    return RunState::Continue;
}
