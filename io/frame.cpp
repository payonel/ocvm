#include "frame.h"
#include "color/color_types.h"

Frame::~Frame()
{
  _screen = nullptr;
}

void Frame::open(IScreen* screen)
{
  _screen = screen;
  _isOn = true;
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

void Frame::write(int x, int y, const Cell& cell, ColorState& cst)
{
  if (cell.locked || !on())
    return;
  onWrite(x, y, cell, cst);
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
  if (_screen)
    _screen->setResolution(width, height);
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

bool Frame::on() const
{
  return _isOn;
}

bool Frame::on(bool bOn)
{
  bool was_changed = _isOn != bOn;
  _isOn = bOn;
  if (was_changed)
  {
    if (bOn)
    {
      _screen->invalidate();
    }
    else
    {
      clear();
    }
  }
  return was_changed;
}
