#include "basic_term.h"
#include <iostream>
using namespace std;

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
    cout << cell.value << flush;
}

tuple<int, int> BasicTerm::maxResolution() const
{
    return std::make_tuple(40, 10);
}
