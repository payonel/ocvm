#include "screen.h"

#include <iostream>
using std::cout;

static const char* esc = "\033[";

void setPos(int x, int y)
{
    cout << esc << y << ";" << x << "f";
}

void up(int num)
{
    cout << esc << num << "A";
}

void down(int num)
{
    cout << esc << num << "B";
}

void right(int num)
{
    cout << esc << num << "C";
}

void left(int num)
{
    cout << esc << num << "D";
}

void clear()
{
    cout << "\033[2J";
}

void clearLine()
{
    cout << "\033[K";
}

void savePos()
{
    cout << "\033[s";
}

void restorePos()
{
    cout << "\033[u";
}

void Screen::test()
{
    cout << "hello world\n";

    clear();
    savePos();
    setPos(10, 10);
    cout << "here";
    setPos(20, 12);
    cout << "here";
    restorePos();
}

void Screen::invoke(const std::string& methodName)
{
    if (methodName == "test")
        this->test();
}
