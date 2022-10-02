#pragma once

#include "io/frame.h"

class BasicTerm : public Frame
{
protected:
  void onWrite(int x, int y, const Cell& cell, ColorState& cst) override;
  tuple<int, int> onOpen() override;
};
