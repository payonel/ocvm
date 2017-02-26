#include "screen.h"

#include <iostream>
#include <vector>

#include "client.h"
#include "host.h"
#include "framing/frame.h"
#include "apis/unicode.h"

Screen::Screen()
{
    add("getKeyboards", &Screen::getKeyboards);
    scrolling(false);
}

bool Screen::onInitialize(Value& config)
{
    // we now have a client and can add ourselves to the framer
    return client()->host()->getFramer()->add(this, 0);
}

int Screen::getKeyboards(lua_State* lua)
{
    Value list = Value::table();
    for (const auto& kb : _keyboards)
    {
        list.insert(kb);
    }
    return ValuePack::ret(lua, list);
}

void Screen::mouse(int btn, int x, int y)
{
}

void Screen::keyboard(bool bPressed, uint keysym, uint keycode)
{
    for (const auto& kbaddr : _keyboards)
    {
        client()->pushSignal({bPressed ? "key_down" : "key_up", kbaddr, keysym, keycode});
    }
}

void Screen::addKeyboard(const string& addr)
{
    _keyboards.push_back(addr);
}
