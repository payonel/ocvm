#include "screen.h"

#include <iostream>
#include <vector>

#include "model/client.h"
#include "model/host.h"
#include "apis/unicode.h"

#include "model/log.h"

#include "keyboard.h"

#include <algorithm>

Screen::Screen()
{
    add("getKeyboards", &Screen::getKeyboards);
    add("getAspectRatio", &Screen::getAspectRatio);
    scrolling(false);

    _mouse = new MouseInput;
}

Screen::~Screen()
{
    delete _mouse;
    _mouse = nullptr;
}

bool Screen::onInitialize()
{
    // we now have a client and can add ourselves to the framer
    if (!client()->host()->getFramer()->add(this, 0))
        return false;

    return true;
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

void Screen::push(KeyEvent ke)
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

void Screen::push(MouseEvent me)
{
    _mouse->push(me);
}

