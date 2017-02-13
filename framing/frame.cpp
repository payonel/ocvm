#include "frame.h"
#include "log.h"
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

    if (index >= 0 && index < _frames.size()) // reorder
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
    _buffer(),
    _width(0),
    _height(0)
{
}

Frame::~Frame(){}

bool Frame::setResolution(int width, int height, bool bQuiet)
{
    if (width > 255 || height > 255)
        return false;

    _width = width;
    _height = height;

    if (_framer && !bQuiet)
    {
        _framer->onResolution(this);
    }

    return true;
}

tuple<int, int> Frame::getResolution() const
{
    return std::make_tuple(_width, _height);
}

void Frame::framer(Framer* pfr)
{
    _framer = pfr;
    if (!_buffer.empty())
    {
        _framer->onWrite(this);
    }
}

Framer* Frame::framer() const
{
    return _framer;
}

void Frame::write(const string& text)
{
    _buffer.push(make_tuple(_x, _y, text));
    if (_framer)
    {
        _framer->onWrite(this);
    }
}

tuple<int, int, string> Frame::pop()
{
    auto next = _buffer.front();
    _buffer.pop();
    return next;
}

bool Frame::empty() const
{
    return _buffer.empty();
}

void Frame::scrolling(bool enable)
{
    _scrolling = enable;
}

bool Frame::scrolling() const
{
    return _scrolling;
}

int Frame::x() const
{
    return _x;
}

int Frame::y() const
{
    return _y;
}

void Frame::move(int x, int y)
{
    _x = x;
    _y = y;
}
