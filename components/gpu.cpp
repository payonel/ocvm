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
    add("get", &Gpu::get);
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

void Gpu::check(lua_State* lua) const
{
    if (!_screen)
    {
        luaL_error(lua, "no screen");
    }
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
        return ValuePack::ret(lua, Value::nil, "invalid address");
    }
    Screen* screen = dynamic_cast<Screen*>(pc);
    if (!screen)
    {
        return ValuePack::ret(lua, Value::nil, "not a screen");
    }
    _screen = screen;

    return 0;
}

int Gpu::setResolution(lua_State* lua)
{
    check(lua);
    int width = (int)Value::check(lua, 1, "number").toNumber();
    int height = (int)Value::check(lua, 2, "number").toNumber();

    tuple<int, int> max = _screen->framer()->maxResolution();
    if (width < 1 || width > std::get<0>(max) ||
        height < 1 || height > std::get<1>(max))
    {
        luaL_error(lua, "unsupported resolution");
        return 0;
    }

    _screen->setResolution(width, height);
    return ValuePack::ret(lua, true);
}

// bool Gpu::set(int x, int y, const string& text)
// {
//     if (!_screen)
//     {
//         return false;
//     }

//     auto rez = _screen->getResolution();
//     int width_available = std::get<0>(rez) - x + 1;

//     string fit = UnicodeApi::sub(text, 1, width_available);
//     while (_buffer.size() < static_cast<size_t>(y))
//         _buffer.push_back("");

//     string line = _buffer.at(y - 1);
//     string newline = UnicodeApi::sub(line, 1, x - 1);
//     auto missing = std::max(static_cast<size_t>(0), x - UnicodeApi::wlen(newline) - 1);
//     newline.insert(0, missing, ' ');
//     newline = newline + fit;
//     newline += UnicodeApi::sub(line, UnicodeApi::wlen(newline) + 1, string::npos);
//     _buffer.at(y - 1) = newline;

//     _screen->move(x, y);
//     _screen->write(fit);

//     return true;
// }

int Gpu::set(lua_State* lua)
{
    check(lua);
    int x = Value::check(lua, 1, "number").toNumber();
    int y = Value::check(lua, 2, "number").toNumber();
    string text = Value::check(lua, 3, "string").toString();
    _screen->set(x, y, text);
    return ValuePack::ret(lua, true);
}

int Gpu::get(lua_State* lua)
{
    check(lua);
    int x = Value::check(lua, 1, "number").toNumber();
    int y = Value::check(lua, 2, "number").toNumber();
    const Cell* pc = _screen->get(x, y);
    if (!pc)
    {
        luaL_error(lua, "index out of bounds");
        return 0;
    }

    Value fgp = pc->fg.paletted ? Value(true) : Value::nil;
    Value bgp = pc->bg.paletted ? Value(true) : Value::nil;

    return ValuePack::ret(lua, pc->value, pc->fg.rgb, pc->bg.rgb, fgp, bgp);
}

int Gpu::maxResolution(lua_State* lua)
{
    check(lua);
    tuple<int, int> res = _screen->framer()->maxResolution();
    return ValuePack::ret(lua, std::get<0>(res), std::get<1>(res));
}

int Gpu::setBackground(lua_State* lua)
{
    check(lua);
    Color old = _screen->background();
    int rgb = Value::check(lua, 1, "number").toNumber();
    bool p = Value::check(lua, 2, "boolean", "nil").Or(false).toBool();
    _screen->background({rgb, p});
    Value vp = old.paletted ? Value(true) : Value::nil;
    lua_settop(lua, 0);
    return ValuePack::ret(lua, old.rgb, vp);
}

int Gpu::getBackground(lua_State* lua)
{
    check(lua);
    const Color& old = _screen->background();
    Value vp = old.paletted ? Value(true) : Value::nil;
    return ValuePack::ret(lua, old.rgb, vp);
}

int Gpu::setForeground(lua_State* lua)
{
    check(lua);
    Color old = _screen->foreground();
    int rgb = Value::check(lua, 1, "number").toNumber();
    bool p = Value::check(lua, 2, "boolean", "nil").Or(false).toBool();
    _screen->foreground({rgb, p});
    Value vp = old.paletted ? Value(true) : Value::nil;
    lua_settop(lua, 0);
    return ValuePack::ret(lua, old.rgb, vp);
}

int Gpu::getForeground(lua_State* lua)
{
    check(lua);
    const Color& old = _screen->foreground();
    Value vp = old.paletted ? Value(true) : Value::nil;
    return ValuePack::ret(lua, old.rgb, vp);
}

int Gpu::fill(lua_State* lua)
{
    check(lua);

    int x = Value::check(lua, 1, "number").toNumber();
    int y = Value::check(lua, 2, "number").toNumber();
    int width = Value::check(lua, 3, "number").toNumber();
    int height = Value::check(lua, 4, "number").toNumber();
    string text = Value::check(lua, 5, "string").toString();

    if (UnicodeApi::len(text) != 1)
    {
        return ValuePack::ret(lua, Value::nil, "invalid fill value");
    }

    text.insert(0, width - 1, text.at(0));

    for (int row = y; row <= height; row++)
    {
        _screen->set(x, row, text);
    }

    return ValuePack::ret(lua, true);
}

int Gpu::copy(lua_State* lua)
{
    check(lua);
    int x = Value::check(lua, 1, "number").toNumber();
    int y = Value::check(lua, 2, "number").toNumber();
    int width = Value::check(lua, 3, "number").toNumber();
    int height = Value::check(lua, 4, "number").toNumber();
    int tx = Value::check(lua, 5, "number").toNumber();
    int ty = Value::check(lua, 6, "number").toNumber();

    auto dim = _screen->getResolution();
    int real_width = std::get<0>(dim);
    int real_height = std::get<1>(dim);

    if (tx > real_width || ty > real_height)
        return ValuePack::ret(lua, true);

    x = std::min(real_width, x);
    y = std::min(real_height, y);

    if (x < 1)
    {
        width += x - 1;
        x = 1;
    }
    if (y < 1)
    {
        height += y - 1;
        y = 1;
    }
    
    width = std::min(real_width - x + 1, width);
    height = std::min(real_height - y + 1, height);

    if (width <= 0 || height <= 0)
        return ValuePack::ret(lua, true);

    vector<vector<const Cell*>> scans;
    for (int yoffset = 0; yoffset < height; yoffset++)
    {
        scans.push_back(_screen->scan(x, y + yoffset, width));
    }

    for (int yoffset = 0; yoffset < height; yoffset++)
    {
        _screen->set(tx, ty + yoffset, scans.at(yoffset));
    }

    return ValuePack::ret(lua, true);
}

int Gpu::getDepth(lua_State* lua)
{
    return ValuePack::ret(lua, 8);
}

int Gpu::getViewport(lua_State* lua)
{
    return maxResolution(lua);
}

int Gpu::getScreen(lua_State* lua)
{
    check(lua);
    return ValuePack::ret(lua, _screen->address());
}
