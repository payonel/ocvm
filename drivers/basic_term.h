#pragma once

#include "io/frame.h"

class BasicTerm : public Frame
{
protected:
  void onWrite(int x, int y, const Cell& cell) override;
  tuple<int, int> onOpen() override;
};
