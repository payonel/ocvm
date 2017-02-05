#include "frame.h"
#include "log.h"
#include <iostream>

using std::string;

Framer::Framer()
{
}

Framer::~Framer()
{
}

bool Framer::add(Frame* pframe)
{
    for (auto pf : _frames)
    {
        if (pf == pframe)
        {
            log << "attempt to add the same frame twice\n";
            return false;
        }
    }

    _frames.push_back(pframe);
    pframe->setFramer(this);

    return true;
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
    if (width < 1 || width > 255 || height < 1 || height > 255)
        return false;

    int oldw = _width;
    int oldh = _height;

    _width = width;
    _height = height;

    if (_framer)
    {
        _framer->onResolution(this, oldw, oldh);
    }
}

void Frame::getResolution(int* pWidth ,int* pHeight)
{
    *pWidth = _width;
    *pHeight = _height;
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

Frame& operator<< (Frame& frame, const string& text)
{
    frame.write(text);
    return frame;
}

Frame& operator<< (Frame& frame, std::ostream& (*pf)(std::ostream&))
{
    frame << "\n";
    return frame;
}