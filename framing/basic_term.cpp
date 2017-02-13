#include "basic_term.h"
#include <iostream>
using namespace std;

bool BasicTerm::update()
{
    cout << "update\n" << endl;
    return true;// cin.get() != 'q';
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
    while (!pWhichFrame->empty())
    {
        cout << std::get<2>(pWhichFrame->pop());
    }
    cout << flush;
}

void BasicTerm::onResolution(Frame* pWhichFrame)
{
}

tuple<int, int> BasicTerm::maxResolution() const
{
    return std::make_tuple(100, 100);
}
