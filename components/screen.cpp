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
    add("getKeyboards", &Screen::getKeyboards);
    add("getAspectRatio", &Screen::getAspectRatio);

    _mouse.reset(new MouseInput);
}

Screen::~Screen()
{
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
    while (_mouse->pop(me))
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

int Screen::getKeyboards(lua_State* lua)
{
    Value list = Value::table();
    for (const auto& kb : _keyboards)
    {
        list.insert(kb->address());
    }
    return ValuePack::ret(lua, list);
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

int Screen::getAspectRatio(lua_State* lua)
{
    return ValuePack::ret(lua, 1, 1);
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
    if (_keyboards.size() == 0)
        return;
    else if (_keyboards.size() == 1)
        _keyboards.at(0)->inputDevice()->push(ke);
    else
    {
        // kb events are duplicated to all kbs
        for (auto* kb : _keyboards)
        {
            kb->inputDevice()->push(ke);
        }
    }
}

void Screen::push(const MouseEvent& me)
{
    _mouse->push(me);
}

Frame* Screen::frame() const
{
    return _frame.get();
}
