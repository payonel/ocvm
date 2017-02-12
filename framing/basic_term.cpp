#include "basic_term.h"
#include <iostream>
using namespace std;

bool BasicTerm::update()
{
    cout << "update\n" << endl;
    return cin.get() != 'q';
}

bool BasicTerm::open()
{
    return true;
}

void BasicTerm::close()
{
}

bool BasicTerm::onAdd(Frame* pf)
{
    return true;
}

void BasicTerm::onWrite(Frame* pWhichFrame)
{
    cout << pWhichFrame->read() << flush;
}

void BasicTerm::onResolution(Frame* pWhichFrame)
{
}

tuple<int, int> BasicTerm::maxResolution() const
{
    return std::make_tuple(100, 100);
}
