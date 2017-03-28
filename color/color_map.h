#pragma once

#include "color_types.h"

class ColorMap
{
public:
    // TODO: load monochrome from settings
    static int monochrome_color;

    static int inflate(int value, EDepthType depth);
    static int deflate(const Color& color, EDepthType depth);

    static void redeflate(Color* pColor, EDepthType old_depth, EDepthType new_depth);
};
