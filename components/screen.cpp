#include "screen.h"

#include <iostream>
#include <vector>

#include "client.h"
#include "host.h"
#include "framing/frame.h"
#include "apis/unicode.h"

Screen::Screen()
{
    add("getKeyboards", &Screen::getKeyboards);
}

bool Screen::onInitialize(Value& config)
{
    // we now have a client and can add ourselves to the framer
    return client()->host()->getFramer()->add(this, 0);
}

int Screen::getKeyboards(lua_State* lua)
{
    Value list = Value::table();
    for (const auto& kb : _keyboards)
    {
        list.insert(kb);
    }
    return ValuePack::push(lua, list);
}

void Screen::resizeBuffer(int width, int height)
{
    bool bad_dim = width <= 0 || height <= 0;
    if (bad_dim || _cells)
    {
        delete [] _cells;
        _cells = nullptr;
    }
    if (bad_dim)
        return;

    size_t size = width * height;
    Cell* ptr = new Cell[size];
    for (size_t i = 0; i < size; i++)
    {
        Cell& c = ptr[i];
        c.value = {};
        c.fg = {};
        c.bg = {};
    }
    _cells = ptr;
}

bool Screen::setResolution(int width, int height, bool bQuiet)
{
    bool bRet = Frame::setResolution(width, height, bQuiet);
    if (bRet)
    {
        resizeBuffer(width, height);
    }
    return bRet;
}

const Cell* Screen::get(int x, int y) const
{
    x--;
    y--;
    auto dim = getResolution();
    int width = std::get<0>(dim);
    int height = std::get<1>(dim);
    if (x < 0 || x >= width || y < 0 || y >= height || _cells == nullptr)
        return nullptr;

    return &_cells[y*width + x];
}

void Screen::set(int x, int y, const Cell& cell)
{
    x--;
    y--;
    auto dim = getResolution();
    int width = std::get<0>(dim);
    int height = std::get<1>(dim);
    if (x < 0 || x >= width || y < 0 || y >= height || _cells == nullptr)
        return;

    _cells[y*width + x] = cell;
    Frame::move(x, y);
    Frame::write(cell.value);
}

void Screen::set(int x, int y, const string& text)
{
    int len = UnicodeApi::len(text);
    for (int i = 1; i <= len; i++)
    {
        set(x + i - 1, y, {UnicodeApi::sub(text, i, i), _fg, _bg});
    }
}

void Screen::set(int x, int y, const vector<const Cell*>& scanned)
{
    for (size_t i = 0; i < scanned.size(); i++)
    {
        const Cell* pc = scanned.at(i);
        if (pc)
            set(x + i, y, *pc);
    }
}

void Screen::foreground(const Color& color)
{
    _fg = color;
}

const Color& Screen::foreground() const
{
    return _fg;
}

void Screen::background(const Color& color)
{
    _bg = color;
}

const Color& Screen::background() const
{
    return _bg;
}

vector<const Cell*> Screen::scan(int x, int y, int width) const
{
    x--;
    y--;
    vector<const Cell*> result;
    for (int i = 0; i < width; i++)
    {
        result.push_back(get(x + i, y));
    }

    return result;
}
