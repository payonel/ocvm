#include "term_buffer.h"
#include "ansi.h"

size_t TermBuffer::size() const
{
  return _data.size();
}

void TermBuffer::push(char ch)
{
  _data.push_back(ch);
}

char TermBuffer::get()
{
  if (size() == 0)
    return 0;
  char ch = _data.at(0);
  _data.pop_front();
  return ch;
}

char TermBuffer::peek(size_t offset) const
{
  if (offset >= size())
    return 0;
  return _data.at(offset);
}

bool TermBuffer::hasMouseCode() const
{
  if (size() > 3 && peek(0) == Ansi::ESC && peek(1) == '[' && peek(2) == 'M')
  {
    return true;
  }
  return false;
}
