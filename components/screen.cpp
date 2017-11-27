#include <iostream>
#include <vector>
#include <algorithm>

#include "screen.h"
#include "gpu.h"
#include "keyboard.h"

#include "model/client.h"
#include "model/host.h"
#include "model/log.h"

#include "apis/unicode.h"

Screen::Screen()
{
    add("isOn", &Screen::isOn);
    add("getAspectRatio", &Screen::getAspectRatio);
    add("getKeyboards", &Screen::getKeyboards);
    add("setTouchModeInverted", &Screen::setTouchModeInverted);
    add("turnOn", &Screen::turnOn);
    add("turnOff", &Screen::turnOff);
    add("isPrecise", &Screen::isPrecise);
    add("isTouchInverted", &Screen::isTouchInverted);
    add("setPrecise", &Screen::setPrecise);
}

///////////////////////////// COMPONENT API /////////////////////////////

int Screen::isOn(lua_State* lua)
{
    return ValuePack::ret(lua, _frame->on());
}

int Screen::getAspectRatio(lua_State* lua)
{
    return ValuePack::ret(lua, 1, 1);
}

int Screen::getKeyboards(lua_State* lua)
{
    Value list = Value::table();
    for (const auto& kb : _keyboards)
    {
        list.insert(kb->address());
    }
    return ValuePack::ret(lua, list);
}

int Screen::setTouchModeInverted(lua_State* lua)
{
    return ValuePack::ret(lua, Value::nil, "not supported");
}

int Screen::turnOn(lua_State* lua)
{
    return ValuePack::ret(lua, _frame->on(true));
}

int Screen::turnOff(lua_State* lua)
{
    return ValuePack::ret(lua, _frame->on(false));
}

int Screen::isPrecise(lua_State* lua)
{
    return ValuePack::ret(lua, false);
}

int Screen::isTouchInverted(lua_State* lua)
{
    return ValuePack::ret(lua, false);
}

int Screen::setPrecise(lua_State* lua)
{
    return ValuePack::ret(lua, Value::nil, "not supported");
}


/////////////////////////////////////////////////////////////////////////

Screen::~Screen()
{
    // make a copy of the vector because detach modifies the vector
    vector<Keyboard*> kb_copy = _keyboards;
    for (auto& kb : kb_copy)
        kb->detach();

    if (_frame)
    {
        _frame->close();
    }
    if (_gpu)
    {
        _gpu->unbind();
    }

    _gpu = nullptr;
}

bool Screen::onInitialize()
{
    // we now have a client and we are ready to create the frame for this screen
    _frame.reset(client()->host()->createFrame());
    if (!_frame)
    {
        return false;
    }
    _frame->open(this);

    return true;
}

void Screen::gpu(Gpu* gpu)
{
    _gpu = gpu;
}

Gpu* Screen::gpu() const
{
    return _gpu;
}

RunState Screen::update()
{
    MouseEvent me;
    while (EventSource<MouseEvent>::pop(me))
    {
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

    if (!_frame->update())
    {
        return RunState::Halt;
    }

    return RunState::Continue;
}

vector<string> Screen::keyboards() const
{
    vector<string> addrs;
    for (const Keyboard* kb : _keyboards)
    {
        addrs.push_back(kb->address());
    }
    return addrs;
}

bool Screen::connectKeyboard(Keyboard* kb)
{
    _keyboards.push_back(kb);
    return true;
}

bool Screen::disconnectKeyboard(Keyboard* kb)
{
    return _keyboards.erase(std::remove(_keyboards.begin(), _keyboards.end(), kb), _keyboards.end()) == _keyboards.end();
}

void Screen::push(const KeyEvent& ke)
{
    // kb events are duplicated to all kbs
    for (auto* kb : _keyboards)
    {
        kb->push(ke);
    }
}

Frame* Screen::frame() const
{
    return _frame.get();
}
