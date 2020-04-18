#pragma once

#include "color_types.h"

class ColorMap
{
public:
  static int inflate(const ColorState& state, int value);
  static int deflate(const ColorState& state, const Color& color);

  static int deflate(int rgb);

  static void initialize_color_state(ColorState& state, EDepthType depth);
  static void set_monochrome(int rgb);
};
