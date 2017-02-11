#pragma once

#include <string>
using std::string;

class Framer;

class FrameFactory
{
public:
    static Framer* create(const string& framerTypeName);
};
