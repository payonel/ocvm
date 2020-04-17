#pragma once

#include <deque>
using std::deque;
using std::size_t;

class TermBuffer
{
public:
  size_t size() const;
  void push(char ch);
  char get();
  char peek(size_t offset = 0) const;
  bool hasMouseCode() const;

private:
  deque<char> _data;
};
