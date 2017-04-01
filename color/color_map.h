#pragma once

#include "color_types.h"

class ColorMap
{
public:
    static int inflate(int value, EDepthType depth);
    static int deflate(const Color& color, EDepthType depth);

    static void redeflate(Color* pColor, EDepthType old_depth, EDepthType new_depth);
    static int deflate(int rgb);

    static void set_monochrome(int rgb);
};
