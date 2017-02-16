#include "gpu.h"
#include "log.h"
#include "client.h"
#include "screen.h"
#include "apis/unicode.h"
#include <iostream>

Gpu::Gpu()
{
    add("setResolution", &Gpu::setResolution);
    add("bind", &Gpu::bind);
    add("set", &Gpu::set);
    add("maxResolution", &Gpu::maxResolution);
    add("getBackground", &Gpu::getBackground);
    add("setBackground", &Gpu::setBackground);
    add("getForeground", &Gpu::getForeground);
    add("setForeground", &Gpu::setForeground);
    add("fill", &Gpu::fill);
    add("copy", &Gpu::copy);
}

bool Gpu::onInitialize(Value& config)
{
    return true;
}

int Gpu::bind(lua_State* lua)
{
    string address = Value::check(lua, 1, "string").toString();
    Component* pc = client()->component(address);
    if (!pc)
    {
        return ValuePack::push(lua, Value::nil, "invalid address");
    }
    Screen* screen = dynamic_cast<Screen*>(pc);
    if (!screen)
    {
        return ValuePack::push(lua, Value::nil, "not a screen");
    }
    _screen = screen;

    return 0;
}

int Gpu::setResolution(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, false, "no screen");
    }
    int width = (int)Value::check(lua, 1, "number").toNumber();
    int height = (int)Value::check(lua, 2, "number").toNumber();

    tuple<int, int> max = _screen->framer()->maxResolution();
    if (width < 1 || width > std::get<0>(max) ||
        height < 1 || height > std::get<1>(max))
    {
        return ValuePack::push(lua, false, "unsupported resolution");
    }

    _screen->setResolution(width, height);
    return ValuePack::push(lua, true);
}

bool Gpu::set(int x, int y, const string& text)
{
    if (!_screen)
    {
        return false;
    }

    _screen->move(x, y);
    _screen->write(text);

    return true;
}

int Gpu::set(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, false, "no screen");
    }
    int x = Value::check(lua, 1, "number").toNumber();
    int y = Value::check(lua, 2, "number").toNumber();
    string text = Value::check(lua, 3, "string").toString();

    return ValuePack::push(lua, set(x, y, text));
}

int Gpu::maxResolution(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, false, "no screen");
    }
    tuple<int, int> res = _screen->framer()->maxResolution();
    return ValuePack::push(lua, std::get<0>(res), std::get<1>(res));
}

int Gpu::setBackground(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, false, "no screen");
    }
    lout << "TODO, stub gpu method\n";
    return ValuePack::push(lua, 0, false);
}

int Gpu::getBackground(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, false, "no screen");
    }
    lout << "TODO, stub gpu method\n";
    return ValuePack::push(lua, 0, false);
}

int Gpu::setForeground(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, false, "no screen");
    }
    lout << "TODO, stub gpu method\n";
    return ValuePack::push(lua, 0, false);
}

int Gpu::getForeground(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, false, "no screen");
    }
    lout << "TODO, stub gpu method\n";
    return ValuePack::push(lua, 0, false);
}

int Gpu::fill(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, false, "no screen");
    }

    int x = Value::check(lua, 1, "number").toNumber();
    int y = Value::check(lua, 2, "number").toNumber();
    int width = Value::check(lua, 3, "number").toNumber();
    int height = Value::check(lua, 4, "number").toNumber();
    string text = Value::check(lua, 5, "string").toString();

    if (!truncateWH(x, y, &width, &height))
    {
        return ValuePack::push(lua, false, "out of bounds");
    }

    if (UnicodeApi::wlen(text) != 1)
    {
        return ValuePack::push(lua, false, "fill char not length 1");
    }

    text.insert(0, width, text.at(0));

    for (int row = y; row <= height; row++)
    {
        set(x, row, text);
    }

    return ValuePack::push(lua, true);
}

int Gpu::copy(lua_State* lua)
{
    if (!_screen)
        return ValuePack::push(lua, false, "no screen");

    int x = Value::check(lua, 1, "number").toNumber();
    int y = Value::check(lua, 2, "number").toNumber();
    int width = Value::check(lua, 3, "number").toNumber();
    int height = Value::check(lua, 4, "number").toNumber();
    int tx = Value::check(lua, 5, "number").toNumber();
    int ty = Value::check(lua, 6, "number").toNumber();

    if (!truncateWH(x, y, &width, &height))
    {
        return ValuePack::push(lua, false, "out of bounds");
    }

    int twidth = width;
    int theight = height;

    if (!truncateWH(tx, ty, &twidth, &theight))
    {
        return ValuePack::push(lua, false, "out of bounds");
    }

    lout << "TODO, stub gpu method\n";
    return ValuePack::push(lua, false, "not implemented");
}

bool Gpu::truncateWH(int x, int y, int* pWidth, int* pHeight) const
{
    if (!_screen || !pWidth || !pHeight)
        return false;

    auto dim = _screen->getResolution();
    int swidth = std::get<0>(dim);
    int sheight = std::get<1>(dim);
    if (x > swidth || y > sheight || x < 1 || y < 1)
        return false;

    // truncate request, maybe this isn't true to oc behavior

    if (*pWidth < 1)
        *pWidth = 1;
    if (*pHeight < 1)
        *pHeight = 1;

    *pWidth = std::min(*pWidth, swidth - x);
    *pHeight = std::min(*pHeight, sheight - y);

    return true;
}
