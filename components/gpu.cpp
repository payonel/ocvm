#include "gpu.h"
#include "log.h"
#include "client.h"
#include "screen.h"
#include "apis/unicode.h"
#include "color/color_map.h"
#include <iostream>

using namespace std;

Gpu::Gpu()
{
    add("getResolution", &Gpu::getResolution);
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
    add("setDepth", &Gpu::setDepth);
    add("getViewport", &Gpu::getViewport);
    add("getScreen", &Gpu::getScreen);
    add("maxDepth", &Gpu::maxDepth);
}

Gpu::~Gpu()
{
}

void Gpu::check(lua_State* lua) const
{
    if (!_screen)
    {
        luaL_error(lua, "no screen");
    }
}

void Gpu::deflate(lua_State* lua, Color* pRawColor)
{
    check(lua);
    EDepthType depth = _screen->getDepth();
    if (pRawColor->paletted)
    {
        if (depth == EDepthType::_1)
        {
            luaL_error(lua, "color palette not supported");
        }
        if (pRawColor->rgb < 0 || (size_t)pRawColor->rgb >= PALETTE_SIZE) // palette size
        {
            luaL_error(lua, "invalid palette index");
        }
    }

    pRawColor->rgb = ColorMap::deflate(*pRawColor, depth);
}

bool Gpu::onInitialize(Value& config)
{
    return true;
}

int Gpu::bind(lua_State* lua)
{
    string address = Value::check(lua, 1, "string").toString();
    if (_screen && _screen->address() == address)
        return 0; // already set

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

int Gpu::maxDepth(lua_State* lua)
{
    return ValuePack::ret(lua, static_cast<int>(EDepthType::_8));
}

int Gpu::getResolution(lua_State* lua)
{
    check(lua);
    auto dim = _screen->getResolution();
    return ValuePack::ret(lua, std::get<0>(dim), std::get<1>(dim));
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

    auto fg_ctx = makeColorContext(pc->fg);
    auto bg_ctx = makeColorContext(pc->bg);

    return ValuePack::ret(lua, pc->value, 
        std::get<0>(fg_ctx), std::get<0>(bg_ctx), 
        std::get<1>(fg_ctx), std::get<1>(bg_ctx));
}

int Gpu::maxResolution(lua_State* lua)
{
    check(lua);
    tuple<int, int> res = _screen->framer()->maxResolution();
    return ValuePack::ret(lua, std::get<0>(res), std::get<1>(res));
}

int Gpu::setBackground(lua_State* lua)
{
    return setColorContext(lua, true);
}

int Gpu::getBackground(lua_State* lua)
{
    return getColorContext(lua, true);
}

int Gpu::setForeground(lua_State* lua)
{
    return setColorContext(lua, false);
}

int Gpu::getForeground(lua_State* lua)
{
    return getColorContext(lua, false);
}

int Gpu::fill(lua_State* lua)
{
    check(lua);

    int x = Value::check(lua, 1, "number").toNumber();
    int y = Value::check(lua, 2, "number").toNumber();
    int width = Value::check(lua, 3, "number").toNumber();
    int height = Value::check(lua, 4, "number").toNumber();
    string text = Value::check(lua, 5, "string").toString();

    string value = UnicodeApi::sub(text, 1, 1);
    if (value != text || value.empty())
    {
        return ValuePack::ret(lua, Value::nil, "invalid fill value");
    }

    Cell fill_cell { text, _screen->foreground(), _screen->background() };

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            _screen->set(x + col, y + row, fill_cell);
        }
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
    int dx = Value::check(lua, 5, "number").toNumber();
    int dy = Value::check(lua, 6, "number").toNumber();

    auto dim = _screen->getResolution();
    int real_width = std::get<0>(dim);
    int real_height = std::get<1>(dim);

    int tx = x + dx;
    int ty = y + dy;

    if (width <= 0 || height <= 0)
        return ValuePack::ret(lua, true);

    if (tx > real_width || ty > real_height)
        return ValuePack::ret(lua, true);

    if ((tx + width) < 1 || (ty + height) < 1)
        return ValuePack::ret(lua, true);

    if (x > real_width || y > real_height)
        return ValuePack::ret(lua, true);

    if ((x + width) < 1 || (y + height) < 1)
        return ValuePack::ret(lua, true);

    vector<vector<const Cell*>> scans;
    for (int yoffset = 0; yoffset < height; yoffset++)
    {
        auto given_scan = _screen->scan(x, y + yoffset, width);
        vector<const Cell*> next;
        for (const auto* pc : given_scan)
        {
            if (pc)
            {
                Cell* pnew = new Cell;
                *pnew = *pc; // copy
                next.push_back(pnew);
            }
            else
            {
                next.push_back(nullptr);
            }
        }
        scans.push_back(next);
    }

    for (int yoffset = 0; yoffset < height; yoffset++)
    {
        _screen->set(tx, ty + yoffset, scans.at(yoffset));
    }

    for (const auto& scan : scans)
    {
        for (const auto& pc : scan)
        {
            delete pc;
        }
    }

    return ValuePack::ret(lua, true);
}

int Gpu::getDepth(lua_State* lua)
{
    check(lua);
    return ValuePack::ret(lua, static_cast<int>(_screen->getDepth()));
}

int Gpu::setDepth(lua_State* lua)
{
    check(lua);
    double dbits = Value::check(lua, 1, "number").toNumber();

    string depth_identifier;
    switch (_screen->getDepth())
    {
        case EDepthType::_1: depth_identifier = "OneBit"; break;
        case EDepthType::_4: depth_identifier = "FourBit"; break;
        case EDepthType::_8: depth_identifier = "EightBit"; break;
    }

    int bits = dbits == 1.0 ? 1 : dbits == 4.0 ? 4 : dbits == 8.0 ? 8 : 0;
    switch (bits)
    {
        case 1: _screen->setDepth(EDepthType::_1); break;
        case 4: _screen->setDepth(EDepthType::_4); break;
        case 8: _screen->setDepth(EDepthType::_8); break;
        default: luaL_error(lua, "invalid depth");
    }

    return ValuePack::ret(lua, depth_identifier);
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

int Gpu::setColorContext(lua_State* lua, bool bBack)
{
    check(lua);
    
    int rgb = Value::check(lua, 1, "number").toNumber();
    bool p = Value::check(lua, 2, "boolean", "nil").Or(false).toBool();

    int stack = getColorContext(lua, bBack);

    Color color {rgb, p};
    deflate(lua, &color);

    if (bBack)
        _screen->background(color);
    else
        _screen->foreground(color);

    return stack;
}

int Gpu::getColorContext(lua_State* lua, bool bBack)
{
    check(lua);
    const Color& color = bBack ? _screen->background() : _screen->foreground();
    auto ctx = makeColorContext(color);

    return ValuePack::ret(lua, std::get<0>(ctx), std::get<1>(ctx));
}

tuple<int, Value> Gpu::makeColorContext(const Color& color)
{
    int value;
    if (color.paletted)
        value = color.rgb;
    else
        value = ColorMap::inflate(color.rgb, _screen->getDepth());
    Value vp = color.paletted ? Value(true) : Value::nil;

    return make_tuple(value, vp);
}
