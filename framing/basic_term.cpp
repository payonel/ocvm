#include "basic_term.h"
#include <iostream>
using namespace std;

bool BasicTerm::update()
{
    return true;// cin.get() != 'q';
}

bool BasicTerm::open()
{
    return true;
}

void BasicTerm::close()
{
}

void BasicTerm::onWrite(Frame* pf, int x, int y, const Cell& cell)
{
    cout << cell.value << flush;
}

void BasicTerm::onResolution(Frame* pWhichFrame)
{
}

tuple<int, int> BasicTerm::maxResolution() const
{
    return std::make_tuple(40, 10);
}
