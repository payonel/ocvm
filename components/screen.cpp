#include "screen.h"

#include <iostream>
#include <vector>

#include "client.h"
#include "host.h"
#include "apis/unicode.h"

#include "io/mouse_input.h"
#include "log.h"

Screen::Screen()
{
    add("getKeyboards", &Screen::getKeyboards);
    scrolling(false);

    _mouse = new MouseInput;
}

Screen::~Screen()
{
    delete _mouse;
    _mouse = nullptr;
}

bool Screen::onInitialize(Value& config)
{
    // we now have a client and can add ourselves to the framer
    if (!client()->host()->getFramer()->add(this, 0))
        return false;

    return _mouse->open(Factory::create_mouse());
}

RunState Screen::update()
{
    unique_ptr<MouseEvent> pe(_mouse->pop());
    if (pe)
    {
        MouseEvent& me = *static_cast<MouseEvent*>(pe.get());
        string msg;
        switch (me.press)
        {
            case EPressType::Press:
                msg = "touch";
            break;
            case EPressType::Drag:
                msg = "drag";
            break;
            case EPressType::Release: break; // release could always be drop
            case EPressType::Drop:
                msg = "drop";
            break;
        }
        if (!msg.empty())
            client()->pushSignal({msg, address(), me.x, me.y, me.btn / 2});
    }

    return RunState::Continue;
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

void Screen::addKeyboard(const string& addr)
{
    _keyboards.push_back(addr);
}

void Screen::onResize(int width, int height)
{
    if (setResolution(width, height))
        client()->pushSignal({"screen_resized", address(), width, height});
}
