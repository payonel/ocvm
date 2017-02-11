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

    pframe->setFramer(this);

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

bool Frame::setResolution(int width, int height)
{
    if (width > 255 || height > 255)
        return false;

    int oldw = _width;
    int oldh = _height;

    _width = width;
    _height = height;

    if (_framer)
    {
        _framer->onResolution(this);
    }
}

tuple<int, int> Frame::getResolution() const
{
    return std::make_tuple(_width, _height);
}

void Frame::setFramer(Framer* pfr)
{
    _framer = pfr;
    if (!_buffer.empty())
    {
        _framer->onWrite(this);
    }
}

void Frame::write(const string& text)
{
    _buffer += text;
    if (_framer)
    {
        _framer->onWrite(this);
    }
}

string Frame::read()
{
    return std::move(_buffer);
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
