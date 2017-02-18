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
    add("getDepth", &Gpu::getDepth);
    add("getViewport", &Gpu::getViewport);
    add("getScreen", &Gpu::getScreen);
}

Gpu::~Gpu()
{
    // std::cerr << _buffer.size() << std::endl;
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
        return ValuePack::push(lua, Value::nil, "no screen");
    }
    int width = (int)Value::check(lua, 1, "number").toNumber();
    int height = (int)Value::check(lua, 2, "number").toNumber();

    tuple<int, int> max = _screen->framer()->maxResolution();
    if (width < 1 || width > std::get<0>(max) ||
        height < 1 || height > std::get<1>(max))
    {
        return ValuePack::push(lua, Value::nil, "unsupported resolution");
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

    auto rez = _screen->getResolution();
    int width_available = std::get<0>(rez) - x + 1;

    string fit = UnicodeApi::sub(text, 1, width_available);
    while (_buffer.size() < static_cast<size_t>(y))
        _buffer.push_back("");

    string line = _buffer.at(y - 1);
    string newline = UnicodeApi::sub(line, 1, x - 1);
    auto missing = std::max(static_cast<size_t>(0), x - UnicodeApi::wlen(newline) - 1);
    newline.insert(0, missing, ' ');
    newline = newline + fit;
    newline += UnicodeApi::sub(line, UnicodeApi::wlen(newline) + 1, string::npos);
    _buffer.at(y - 1) = newline;

    _screen->move(x, y);
    _screen->write(fit);

    return true;
}

int Gpu::set(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, Value::nil, "no screen");
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
        return ValuePack::push(lua, Value::nil, "no screen");
    }
    tuple<int, int> res = _screen->framer()->maxResolution();
    return ValuePack::push(lua, std::get<0>(res), std::get<1>(res));
}

int Gpu::setBackground(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, Value::nil, "no screen");
    }
    int obg = _bg;
    bool obgp = _bgp;
    _bg = Value::check(lua, 1, "number").toNumber();
    _bgp = Value::check(lua, 2, "boolean", "nil").Or(false).toBool();
    return ValuePack::push(lua, obg, obgp);
}

int Gpu::getBackground(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, Value::nil, "no screen");
    }
    return ValuePack::push(lua, _bg, _bgp);
}

int Gpu::setForeground(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, Value::nil, "no screen");
    }
    int ofg = _fg;
    bool ofgp = _fgp;
    _fg = Value::check(lua, 1, "number").toNumber();
    _fgp = Value::check(lua, 2, "boolean", "nil").Or(false).toBool();
    return ValuePack::push(lua, ofg, ofgp);
}

int Gpu::getForeground(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, Value::nil, "no screen");
    }
    return ValuePack::push(lua, _fg, _fgp);
}

int Gpu::fill(lua_State* lua)
{
    if (!_screen)
    {
        return ValuePack::push(lua, Value::nil, "no screen");
    }

    int x = Value::check(lua, 1, "number").toNumber();
    int y = Value::check(lua, 2, "number").toNumber();
    int width = Value::check(lua, 3, "number").toNumber();
    int height = Value::check(lua, 4, "number").toNumber();
    string text = Value::check(lua, 5, "string").toString();

    if (!truncateWH(x, y, &width, &height))
    {
        return ValuePack::push(lua, Value::nil, "out of bounds");
    }

    if (UnicodeApi::wlen(text) != 1)
    {
        return ValuePack::push(lua, Value::nil, "fill char not length 1");
    }

    text.insert(0, width - 1, text.at(0));

    for (int row = y; row <= height; row++)
    {
        set(x, row, text);
    }

    return ValuePack::push(lua, true);
}

int Gpu::copy(lua_State* lua)
{
    if (!_screen)
        return ValuePack::push(lua, Value::nil, "no screen");

    int x = Value::check(lua, 1, "number").toNumber();
    int y = Value::check(lua, 2, "number").toNumber();
    int width = Value::check(lua, 3, "number").toNumber();
    int height = Value::check(lua, 4, "number").toNumber();
    int tx = Value::check(lua, 5, "number").toNumber();
    int ty = Value::check(lua, 6, "number").toNumber();

    if (!truncateWH(x, y, &width, &height))
    {
        return ValuePack::push(lua, Value::nil, "out of bounds");
    }

    int twidth = width;
    int theight = height;

    if (!truncateWH(tx, ty, &twidth, &theight))
    {
        return ValuePack::push(lua, Value::nil, "out of bounds");
    }

    lout << "TODO, stub gpu method\n";
    return ValuePack::push(lua, Value::nil, "not implemented");
}

int Gpu::getDepth(lua_State* lua)
{
    return ValuePack::push(lua, 8);
}

int Gpu::getViewport(lua_State* lua)
{
    return maxResolution(lua);
}

int Gpu::getScreen(lua_State* lua)
{
    if (!_screen)
        return ValuePack::push(lua, Value::nil, "no screen");
    return ValuePack::push(lua, _screen->address());
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
        *pWidth = 0;
    if (*pHeight < 1)
        *pHeight = 0;

    *pWidth = std::min(*pWidth, swidth - x + 1);
    *pHeight = std::min(*pHeight, sheight - y + 1);

    return true;
}
