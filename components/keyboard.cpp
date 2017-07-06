#include "keyboard.h"
#include "screen.h"
#include "model/client.h"
#include "model/log.h"

#include "io/kb_input.h"

Keyboard::Keyboard()
{
    _keyboard = new KeyboardInput;
}

Keyboard::~Keyboard()
{
    detach();
    delete _keyboard;
    _keyboard = nullptr;
}

void Keyboard::detach()
{
    Screen* pScreen = screen();
    if (pScreen)
    {
        pScreen->disconnectKeyboard(this);
    }
}

bool Keyboard::onInitialize()
{
    _preferredScreen = config().get(ConfigIndex::ScreenAddress).Or("").toString();
    return true;
}

bool Keyboard::postInit()
{
    for (auto* pc : client()->components("screen", true))
    {
        Screen* screen = dynamic_cast<Screen*>(pc);
        if (screen && (_preferredScreen.empty() || _preferredScreen == screen->address()))
        {
            return screen->connectKeyboard(this);
        }
    }

    lout << "warning: kb had no screen to join\n";
    return true;
}

RunState Keyboard::update()
{
    KeyEvent ke;
    while (_keyboard->pop(ke))
    {
        if (ke.keycode == 1)
        {
            lout << "shell abort";
            return RunState::Halt;
        }
        else if (ke.insert.size())
        {
            client()->pushSignal({"clipboard", address(), ke.insert});
        }
        else
        {
            client()->pushSignal({ke.bPressed ? "key_down" : "key_up", address(), ke.keysym, ke.keycode});
        }
    }

    return RunState::Continue;
}

Screen* Keyboard::screen() const
{
    for (auto* pc : client()->components("screen", true))
    {
        Screen* screen = dynamic_cast<Screen*>(pc);
        for (const auto& kb_addr : screen->keyboards())
        {
            if (kb_addr == address())
            {
                return screen;
            }
        }
    }
    return nullptr;
}

KeyboardInput* Keyboard::inputDevice() const
{
    return _keyboard;
}
