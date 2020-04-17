#include "basic_term.h"
#include <iostream>

void BasicTerm::onWrite(int x, int y, const Cell& cell)
{
  std::cout << cell.value;
}

tuple<int, int> BasicTerm::onOpen()
{
  return std::make_tuple(40, 10);
}
