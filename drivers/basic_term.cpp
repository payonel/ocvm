#include "basic_term.h"
#include <iostream>

bool BasicTerm::update()
{
    return true;// cin.get() != 'q';
}

bool BasicTerm::onOpen()
{
    return true;
}

void BasicTerm::onClose()
{
}

void BasicTerm::write(Frame* pf, int x, int y, const Cell& cell)
{
    std::cout << cell.value << std::flush;
}

tuple<int, int> BasicTerm::maxResolution() const
{
    return std::make_tuple(40, 10);
}
