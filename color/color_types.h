#pragma once

#include <cstdlib>

struct Color
{
    int rgb;
    bool paletted;
    unsigned code;
};

constexpr std::size_t PALETTE_SIZE = 16;

enum class EDepthType
{
    _1 = 1,
    _4 = 4,
    _8 = 8
};

