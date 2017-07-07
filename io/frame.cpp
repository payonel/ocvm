#include "frame.h"
#include "components/screen.h"
#include "components/gpu.h"

Frame::~Frame()
{
    _screen = nullptr;
}

void Frame::open(Screen* screen)
{
    _screen = screen;
    auto rez = onOpen();
    _width = std::get<0>(rez);
    _height = std::get<1>(rez);
}

bool Frame::update()
{
    if (!_screen)
        return false;

    onUpdate();
    return true;
}

void Frame::close()
{
    onClose();
    _screen = nullptr;
}

void Frame::write(int x, int y, const Cell& cell)
{
    if (!cell.locked)
    {
        onWrite(x, y, cell);
    }
}

tuple<int, int> Frame::size() const
{
    return std::make_tuple(_width, _height);
}

void Frame::clear()
{
    onClear();
}

void Frame::winched(int width, int height)
{
    _width = width;
    _height = height;
    if (_screen && _screen->gpu())
        _screen->gpu()->setResolution(width, height);
}

void Frame::mouseEvent(const MouseEvent& me)
{
    if (_screen)
        _screen->push(me);
}

void Frame::keyEvent(const KeyEvent& ke)
{
    if (_screen)
        _screen->push(ke);
}
