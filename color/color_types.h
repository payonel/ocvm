#pragma once

#include <cstdlib>

struct Color
{
  int rgb;
  bool paletted;
  unsigned code;
};

namespace Colors
{
const Color Black{ 0x000000 };
const Color White{ 0xffffff };
};

enum class EDepthType
{
  _1 = 1,
  _4 = 4,
  _8 = 8
};

struct ColorState
{
  enum
  {
    PALETTE_SIZE = 16
  };
  int palette[PALETTE_SIZE];
  EDepthType depth;
};
