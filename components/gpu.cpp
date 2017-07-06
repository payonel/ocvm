#include "gpu.h"
#include "model/log.h"
#include "model/client.h"
#include "screen.h"
#include "apis/unicode.h"
#include "color/color_map.h"
#include <iostream>

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
    unbind();
    delete [] _cells;
    _cells = nullptr;
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
    string address = Value::checkArg<string>(lua, 1);
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
        _screen->gpu(nullptr);
    }
    _screen = screen;
    _screen->gpu(this);
    ColorMap::initialize_color_state(_color_state, _screen->frame()->depth());

    tuple<int, int> max = _screen->frame()->size();
    setResolution(std::get<0>(max), std::get<1>(max));

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
    int width = Value::checkArg<int>(lua, 1);
    int height = Value::checkArg<int>(lua, 2);

    tuple<int, int> max = _screen->frame()->size();
    if (width < 1 || width > std::get<0>(max) || height < 1 || height > std::get<1>(max))
    {
        return luaL_error(lua, "unsupported resolution");
    }

    return ValuePack::ret(lua, setResolution(width, height));
}

bool Gpu::setResolution(int width, int height)
{
    if (!_screen || !_screen->frame())
        return false;

    if (width == _width && height == _height)
        return false;

    resizeBuffer(width, height);
    client()->pushSignal({"screen_resized", _screen->address(), width, height});
    return true;
}

int Gpu::set(lua_State* lua)
{
    check(lua);
    int x = Value::checkArg<int>(lua, 1);
    int y = Value::checkArg<int>(lua, 2);
    vector<char> text = Value::checkArg<vector<char>>(lua, 3);

    static const bool default_vertical = false;
    bool bVertical = Value::checkArg<bool>(lua, 4, &default_vertical);

    set(x, y, text, bVertical);
    return ValuePack::ret(lua, true);
}

int Gpu::get(lua_State* lua)
{
    check(lua);
    int x = Value::checkArg<int>(lua, 1);
    int y = Value::checkArg<int>(lua, 2);
    const Cell* pc = get(x, y);
    if (!pc)
    {
        luaL_error(lua, "index out of bounds");
        return 0;
    }

    // palette colors are nil unless set as palette colors
    // unless depth is 4, then palette indexes are always interpretted

    auto fg_ctx = makeColorContext(pc->fg);
    auto bg_ctx = makeColorContext(pc->bg);

    return ValuePack::ret(lua, pc->value, 
        std::get<0>(fg_ctx), std::get<0>(bg_ctx), 
        std::get<1>(fg_ctx), std::get<1>(bg_ctx));
}

int Gpu::maxResolution(lua_State* lua)
{
    check(lua);
    tuple<int, int> res = _screen->frame()->size();
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

    int x = Value::checkArg<int>(lua, 1);
    int y = Value::checkArg<int>(lua, 2);
    int width = Value::checkArg<int>(lua, 3);
    int height = Value::checkArg<int>(lua, 4);
    vector<char> text = Value::checkArg<vector<char>>(lua, 5);

    vector<char> value = UnicodeApi::sub(text, 1, 1);
    if (value.size() != text.size() || value.empty())
    {
        return ValuePack::ret(lua, Value::nil, "invalid fill value");
    }

    Cell fill_cell
    {
        UnicodeApi::toString(text),
        deflate(_fg),
        deflate(_bg),
        false, // not locked
        UnicodeApi::charWidth(value, true)
    };

    for (int row = 0; row < height; row++)
    {
        int x_start = x;
        for (int col = 0; col < width; col++)
        {
            x_start += set(x_start, y + row, fill_cell, false);
        }
    }

    return ValuePack::ret(lua, true);
}

int Gpu::copy(lua_State* lua)
{
    check(lua);
    int x = Value::checkArg<int>(lua, 1);
    int y = Value::checkArg<int>(lua, 2);
    int width = Value::checkArg<int>(lua, 3);
    int height = Value::checkArg<int>(lua, 4);
    int dx = Value::checkArg<int>(lua, 5);
    int dy = Value::checkArg<int>(lua, 6);

    int xadj = std::max(1, x) - x;
    int yadj = std::max(1, y) - y;

    width -= xadj;
    height -= yadj;
    dx -= xadj;
    dy -= yadj;
    x += xadj;
    y += yadj;

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

    if (dx == 0 && dy == 0)
        return ValuePack::ret(lua, true); // might this unlock cells?

    // truncate width and height so our cells are always non-null
    width = std::min(width, _width - x + 1);
    height = std::min(height, _height - y + 1);

    vector<Cell> buffer;
    buffer.reserve(width * height);

    for (int yoffset = 0; yoffset < height; yoffset++)
    {
        Cell* pLine = at(x, y + yoffset);
        for (int xoffset = 0; xoffset < width; xoffset++)
        {
            buffer.push_back(*pLine++);
        }
    }
    Cell* pScan = buffer.data();

    for (int yoffset = 0; yoffset < height; yoffset++)
    {
        for (int xoffset = 0; xoffset < width; xoffset++)
        {
            set(tx + xoffset, ty + yoffset, *pScan++, true);
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
    int bits = Value::checkArg<int>(lua, 1);

    string depth_identifier;
    switch (_color_state.depth)
    {
        case EDepthType::_1: depth_identifier = "OneBit"; break;
        case EDepthType::_4: depth_identifier = "FourBit"; break;
        case EDepthType::_8: depth_identifier = "EightBit"; break;
    }

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
    if (!_screen)
        return ValuePack::ret(lua, Value::nil);
    return ValuePack::ret(lua, _screen->address());
}

int Gpu::setColorContext(lua_State* lua, bool bBack)
{
    check(lua);

    static const bool default_paletted = false;
    int rgb = Value::checkArg<int>(lua, 1);
    bool p = Value::checkArg<bool>(lua, 2, &default_paletted);

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
    return ValuePack::ret(lua, color.rgb, color.paletted);
}

tuple<int, Value> Gpu::makeColorContext(const Color& color)
{
    int value = color.rgb;
    if (!color.paletted || _color_state.depth != EDepthType::_8)
    {
        value = ColorMap::inflate(_color_state, value);
    }

    Value vp = Value::nil;
    if (color.paletted || _color_state.depth == EDepthType::_4)
    {
        vp = color.rgb;
    }

    return std::make_tuple(value, vp);
}

int Gpu::getPaletteColor(lua_State* lua)
{
    int index = Value::checkArg<int>(lua, 1);

    if (_color_state.depth == EDepthType::_1)
        return ValuePack::ret(lua, Value::nil, "palette not available");

    if (index < 0 || index > 15)
        return luaL_error(lua, "invalid palette index");

    return ValuePack::ret(lua, _color_state.palette[index]);
}

int Gpu::setPaletteColor(lua_State* lua)
{
    int index = Value::checkArg<int>(lua, 1);
    int rgb = Value::checkArg<int>(lua, 2);

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

Cell* Gpu::at(int x, int y) const
{
    // positions are 1-based
    if (x < 1 || x > _width || y < 1 || y > _height || _cells == nullptr)
        return nullptr;

    return &_cells[(y-1)*_width + (x-1)];
}

const Cell* Gpu::get(int x, int y) const
{
    return const_cast<const Cell*>(at(x, y));
}

int Gpu::set(int x, int y, const Cell& cell, bool bForce)
{
    Cell* pCell = at(x, y);
    int char_width = cell.width;

    if (pCell && (bForce || !pCell->locked))
    {
        int distance_to_edge = _width - x + 1; // 1-based, _width is inclusive
        if (char_width <= distance_to_edge || bForce)
        {
            if (char_width > 1)
            {
                set(x + 1, y, {" ", cell.fg, cell.bg, true, 1}, bForce);
            }
            else if (pCell->width > 1)
            {
                // unlock next
                Cell* pNext = at(x + 1, y);
                if (pNext)
                    pNext->locked = false;
            }
            if (_screen)
                _screen->frame()->write(x, y, cell);
            *pCell = cell;
        }
    }

    return char_width;
}

void Gpu::set(int x, int y, const vector<char>& text, bool bVertical)
{
    Color deflated_fg = deflate(_fg);
    Color deflated_bg = deflate(_bg);

    for (const auto& sub : UnicodeApi::subs(text))
    {
        int width = set(x, y,
            {
                UnicodeApi::toString(sub),
                deflated_fg,
                deflated_bg,
                false,
                UnicodeApi::charWidth(sub, true)
            }, false);
        if (!bVertical)
            x += width;
        else
            y++;
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
                    it->value = {" "};
                    it->fg = {};
                    it->bg = {};
                    it->locked = false;
                    it->width = 1;
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
    if (!_screen)
        return;

    _screen->frame()->clear();

    for (int y = 1; y <= _height; y++)
    {
        for (int x = 1; x <= _width; x++)
        {
            _screen->frame()->write(x, y, *get(x, y));
        }
    }
}

void Gpu::unbind()
{
    if (_screen)
    {
        _screen->gpu(nullptr);
    }
    _screen = nullptr;
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
