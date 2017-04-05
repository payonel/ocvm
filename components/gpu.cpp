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
    add("setViewport", &Gpu::setViewport);
    add("getScreen", &Gpu::getScreen);
    add("maxDepth", &Gpu::maxDepth);
    add("getPaletteColor", &Gpu::getPaletteColor);
    add("setPaletteColor", &Gpu::setPaletteColor);
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

bool Gpu::onInitialize()
{
    ColorMap::set_monochrome(config().get(ConfigIndex::MonochromeColor).Or(0xffffff).toNumber());
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
    if (_screen) // previously a different screen
    {
        _screen->set_gpu(nullptr);
    }
    _screen = screen;
    _screen->set_gpu(this);
    ColorMap::initialize_color_state(_color_state, _screen->framer()->getInitialDepth());

    return 0;
}

int Gpu::maxDepth(lua_State* lua)
{
    return ValuePack::ret(lua, static_cast<int>(EDepthType::_8));
}

int Gpu::getResolution(lua_State* lua)
{
    check(lua);
    return ValuePack::ret(lua, _width, _height);
}

int Gpu::setResolution(lua_State* lua)
{
    check(lua);
    int width = (int)Value::check(lua, 1, "number").toNumber();
    int height = (int)Value::check(lua, 2, "number").toNumber();

    tuple<int, int> max = _screen->framer()->maxResolution();
    if (width < 1 || width > std::get<0>(max) || height < 1 || height > std::get<1>(max))
    {
        return luaL_error(lua, "unsupported resolution");
    }

    setResolution(width, height);
    return ValuePack::ret(lua, true);
}

void Gpu::setResolution(int width, int height)
{
    if (!_screen || !_screen->framer())
        return;

    if (width == _width && height == _height)
        return;

    resizeBuffer(width, height);
    client()->pushSignal({"screen_resized", _screen->address(), width, height});
}

int Gpu::set(lua_State* lua)
{
    check(lua);
    int x = Value::check(lua, 1, "number").toNumber();
    int y = Value::check(lua, 2, "number").toNumber();
    string text = Value::check(lua, 3, "string").toString();
    bool bVertical = Value::check(lua, 4, "boolean", "nil").Or(false).toBool();

    set(x, y, text, bVertical);
    return ValuePack::ret(lua, true);
}

int Gpu::get(lua_State* lua)
{
    check(lua);
    int x = Value::check(lua, 1, "number").toNumber();
    int y = Value::check(lua, 2, "number").toNumber();
    const Cell* pc = get(x, y);
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

    Cell fill_cell { text, deflate(_fg), deflate(_bg) };

    for (int row = 0; row < height; row++)
    {
        for (int col = 0; col < width; col++)
        {
            set(x + col, y + row, fill_cell);
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

    int tx = x + dx;
    int ty = y + dy;

    if (width <= 0 || height <= 0)
        return ValuePack::ret(lua, true);

    if (tx > _width || ty > _height)
        return ValuePack::ret(lua, true);

    if ((tx + width) < 1 || (ty + height) < 1)
        return ValuePack::ret(lua, true);

    if (x > _width || y > _height)
        return ValuePack::ret(lua, true);

    if ((x + width) < 1 || (y + height) < 1)
        return ValuePack::ret(lua, true);

    vector<vector<const Cell*>> scans;
    for (int yoffset = 0; yoffset < height; yoffset++)
    {
        auto given_scan = scan(x, y + yoffset, width);
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
        set(tx, ty + yoffset, scans.at(yoffset));
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
    return ValuePack::ret(lua, static_cast<int>(_color_state.depth));
}

int Gpu::setDepth(lua_State* lua)
{
    check(lua);
    double dbits = Value::check(lua, 1, "number").toNumber();

    string depth_identifier;
    switch (_color_state.depth)
    {
        case EDepthType::_1: depth_identifier = "OneBit"; break;
        case EDepthType::_4: depth_identifier = "FourBit"; break;
        case EDepthType::_8: depth_identifier = "EightBit"; break;
    }

    int bits = dbits == 1.0 ? 1 : dbits == 4.0 ? 4 : dbits == 8.0 ? 8 : 0;
    EDepthType newDepth;
    switch (bits)
    {
        case 1: newDepth = EDepthType::_1; break;
        case 4: newDepth = EDepthType::_4; break;
        case 8: newDepth = EDepthType::_8; break;
        default: return luaL_error(lua, "invalid depth");
    }

    if (_color_state.depth != newDepth)
    {
        // refresh screen (reinflate and deflate all cells)
        inflate_all();
        ColorMap::initialize_color_state(_color_state, newDepth);
        deflate_all();
        invalidate();
    }

    return ValuePack::ret(lua, depth_identifier);
}

int Gpu::getViewport(lua_State* lua)
{
    return getResolution(lua);
}

// setting viewport must always fit <= resolution
// setting resolution always resets viewport
// returns true if changed
// setting viewport > resolution throws unsupported size
int Gpu::setViewport(lua_State* lua)
{
    return setResolution(lua);
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

    if (p)
    {
        if (_color_state.depth == EDepthType::_1)
        {
            luaL_error(lua, "color palette not supported");
        }
        if (rgb < 0 || (size_t)rgb >= ColorState::PALETTE_SIZE) // palette size
        {
            luaL_error(lua, "invalid palette index");
        }
    }

    Color color {rgb, p};

    int stack = getColorContext(lua, bBack);

    if (bBack)
        _bg = color;
    else
        _fg = color;

    return stack;
}

int Gpu::getColorContext(lua_State* lua, bool bBack)
{
    check(lua);
    const Color& color = bBack ? _bg : _fg;
    Value vp = color.paletted ? Value(true) : Value::nil;

    return ValuePack::ret(lua, color.rgb, vp);
}

tuple<int, Value> Gpu::makeColorContext(const Color& color)
{
    int value = color.rgb;
    if (!color.paletted)
        value = ColorMap::inflate(_color_state, value);
    Value vp = color.paletted ? Value(true) : Value::nil;

    return make_tuple(value, vp);
}

int Gpu::getPaletteColor(lua_State* lua)
{
    int index = Value::check(lua, 1, "number").toNumber();
    if (_color_state.depth == EDepthType::_1)
        return ValuePack::ret(lua, Value::nil, "palette not available");
    if (index < 0 || index > 15)
        return luaL_error(lua, "invalid palette index");
    
    return ValuePack::ret(lua, _color_state.palette[index]);
}

int Gpu::setPaletteColor(lua_State* lua)
{
    int index = Value::check(lua, 1, "number").toNumber();
    int rgb = Value::check(lua, 2, "number").toNumber();
    if (_color_state.depth == EDepthType::_1)
        return ValuePack::ret(lua, Value::nil, "palette not available");
    if (index < 0 || index > 15)
        return luaL_error(lua, "invalid palette index");
    int prev = _color_state.palette[index];
    if (prev != rgb)
    {
        // refresh screen (reinflate and deflate all cells)
        inflate_all();
        _color_state.palette[index] = rgb;
        deflate_all();
        invalidate();
    }
    return ValuePack::ret(lua, prev);
}

const Cell* Gpu::get(int x, int y) const
{
    // positions are 1-based
    if (x < 1 || x > _width || y < 1 || y > _height || _cells == nullptr)
        return nullptr;

    return &_cells[(y-1)*_width + (x-1)];
}

void Gpu::set(int x, int y, const Cell& cell)
{
    // positions are 1-based
    if (x < 1 || x > _width || y < 1 || y > _height || _cells == nullptr)
        return;

    _cells[(y-1)*_width + (x-1)] = cell;
    if (_screen)
        _screen->write(x, y, cell);
}

void Gpu::set(int x, int y, const string& text, bool bVertical)
{
    Color deflated_fg = deflate(_fg);
    Color deflated_bg = deflate(_bg);

    int x_step = bVertical ? 0 : 1;
    int y_step = bVertical ? 1 : 0;

    for (const auto& sub : UnicodeApi::subs(text))
    {
        int step = UnicodeApi::charWidth(sub, true);
        set(x, y, {sub, deflated_fg, deflated_bg});
        x += x_step;
        y += y_step;

        int start_x = x;
        while (--step > 0)
            set(start_x++, y, {" ", deflated_fg, deflated_bg});
        if (!bVertical)
            x = start_x;
    }
}

void Gpu::set(int x, int y, const vector<const Cell*>& scanned)
{
    for (size_t i = 0; i < scanned.size(); i++)
    {
        const Cell* pc = scanned.at(i);
        if (pc)
        {
            set(x + i, y, *pc);
        }
    }
}

vector<const Cell*> Gpu::scan(int x, int y, int width) const
{
    vector<const Cell*> result;
    for (int i = 0; i < width; i++)
    {
        const Cell* pCell = get(x + i, y);
        result.push_back(pCell);
    }

    return result;
}

void Gpu::resizeBuffer(int width, int height)
{
    Cell* ptr = nullptr;
    if (width > 0 && height > 0)
    {
        size_t size = width * height;
        ptr = new Cell[size];
        Cell* it = ptr;
        for (int y = 0; y < height; y++)
        {
            for (int x = 0; x < width; x++)
            {
                const auto& prev_cell = get(x + 1, y + 1); // positions are 1-based
                if (prev_cell)
                {
                    *it = *prev_cell;
                }
                else
                {
                    it->value = {};
                    it->fg = {};
                    it->bg = {};
                }
                it++;
            }
        }
    }
    else
    {
        _width = 0;
        _height = 0;
    }

    if (_cells)
    {
        delete [] _cells;
    }

    _width = width;
    _height = height;
    _cells = ptr;
}

void Gpu::invalidate()
{
    if (!_screen || !_screen->framer())
        return;

    _screen->framer()->clear();

    for (int y = 1; y <= _height; y++)
    {
        for (int x = 1; x <= _width; x++)
        {
            _screen->write(x, y, *get(x, y));
        }
    }
}

void Gpu::winched(int width, int height)
{
    setResolution(width, height);
}

Color Gpu::deflate(const Color& color)
{
    Color deflated = color;
    deflated.rgb = ColorMap::deflate(_color_state, color);
    deflated.code = encode(deflated.rgb);
    return deflated;
}

unsigned char Gpu::encode(int rgb)
{
    if (_color_state.depth == EDepthType::_8)
    {
        return static_cast<unsigned char>(rgb & 0xFF);
    }

    rgb = ColorMap::inflate(_color_state, rgb);
    return static_cast<unsigned char>(ColorMap::deflate(rgb) & 0xFF);
}

void Gpu::inflate_all()
{
    size_t size = _width * _height;
    for (size_t i = 0; i < size; i++)
    {
        auto& cell = _cells[i];
        cell.fg.rgb = ColorMap::inflate(_color_state, cell.fg.rgb);
        cell.bg.rgb = ColorMap::inflate(_color_state, cell.bg.rgb);
    }
}

void Gpu::deflate_all()
{
    size_t size = _width * _height;
    for (size_t i = 0; i < size; i++)
    {
        auto& cell = _cells[i];
        cell.fg = deflate(cell.fg);
        cell.bg = deflate(cell.bg);
    }
}
