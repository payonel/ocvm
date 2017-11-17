#include "keyboard.h"
#include "screen.h"
#include "model/client.h"
#include "model/log.h"

Keyboard::Keyboard()
{
}

Keyboard::~Keyboard()
{
    detach();
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

    lout() << "warning: kb had no screen to join\n";
    return true;
}

RunState Keyboard::update()
{
    KeyEvent ke;
    while (EventSource<KeyEvent>::pop(ke))
    {
        if (ke.insert.size())
        {
            client()->pushSignal({"clipboard", address(), ke.insert});
        }
        else if (ke.keycode == 1)
        {
            lout() << "shell abort";
            return RunState::Halt;
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
