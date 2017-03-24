#include "frame.h"
#include "log.h"
#include "apis/unicode.h"
#include <iostream>

Framer::Framer()
{
}

Framer::~Framer()
{
}

bool Framer::add(Frame* pframe, size_t index)
{
    for (auto pf : _frames)
    {
        if (pf == pframe)
        {
            lout << "attempt to add the same frame twice\n";
            return false;
        }
    }

    if (index < _frames.size()) // reorder
    {
        _frames.insert(_frames.begin() + index, pframe);
    }
    else
    {
        _frames.push_back(pframe);
    }

    pframe->framer(this);

    return onAdd(pframe);
}

Frame::Frame() :
    _framer(nullptr),
    _width(0),
    _height(0)
{
}

Frame::~Frame(){}

bool Frame::setResolution(int width, int height, bool bQuiet)
{
    if (width < 0 || height < 0)
        return false;
    if (width == _width && height == _height)
        return false;

    resizeBuffer(width, height);

    if (_framer && !bQuiet)
    {
        _framer->onResolution(this);
    }

    return true;
}

void Framer::invalidate(Frame* pf, int x, int y)
{
    const Cell* pCell = pf->get(x, y);
    if (pCell)
    {
        onWrite(pf, x, y, *pCell);
    }
}

bool Framer::open()
{
    return onOpen();
}

void Framer::close()
{
    onClose();
    _frames.clear();
}

tuple<int, int> Frame::getResolution() const
{
    return std::make_tuple(_width, _height);
}

void Frame::framer(Framer* pfr)
{
    _framer = pfr;
    _framer->invalidate(this);
}

Framer* Frame::framer() const
{
    return _framer;
}

void Frame::scrolling(bool enable)
{
    _scrolling = enable;
}

bool Frame::scrolling() const
{
    return _scrolling;
}

const Cell* Frame::get(int x, int y) const
{
    // positions are 1-based
    if (x < 1 || x > _width || y < 1 || y > _height || _cells == nullptr)
        return nullptr;

    return &_cells[(y-1)*_width + (x-1)];
}

void Frame::set(int x, int y, const Cell& cell)
{
    // positions are 1-based
    if (x < 1 || x > _width || y < 1 || y > _height || _cells == nullptr)
        return;

    _cells[(y-1)*_width + (x-1)] = cell;
    if (_framer)
        _framer->invalidate(this, x, y);
}

void Frame::set(int x, int y, const string& text)
{
    int i = 0;
    for (const auto& sub : UnicodeApi::subs(text))
    {
        set(x + i++, y, {sub, _fg, _bg});
    }
}

void Frame::set(int x, int y, const vector<const Cell*>& scanned)
{
    for (size_t i = 0; i < scanned.size(); i++)
    {
        const Cell* pc = scanned.at(i);
        if (pc)
            set(x + i, y, *pc);
    }
}

void Frame::foreground(const Color& color)
{
    _fg = color;
}

const Color& Frame::foreground() const
{
    return _fg;
}

void Frame::background(const Color& color)
{
    _bg = color;
}

const Color& Frame::background() const
{
    return _bg;
}

vector<const Cell*> Frame::scan(int x, int y, int width) const
{
    vector<const Cell*> result;
    for (int i = 0; i < width; i++)
    {
        const Cell* pCell = get(x + i, y);
        result.push_back(pCell);
    }

    return result;
}

void Frame::resizeBuffer(int width, int height)
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
                const auto& prev_cell = get(x, y);
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
            }
            it++;
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

