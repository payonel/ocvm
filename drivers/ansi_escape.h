#pragma once

#include "io/frame.h"
#include "raw_tty.h"

#include <tuple>

class AnsiEscapeTerm : public Frame
{
public:
  AnsiEscapeTerm();
  virtual ~AnsiEscapeTerm();

protected:
  void onWrite(int x, int y, const Cell& cell, ColorState& cst) override;
  virtual tuple<int, int> onOpen() override;
  void onUpdate() override;
  void onClose() override;
  void onClear() override;

private:
  string scrub(const string& value) const;

  int _x = 1;
  int _y = 1;
  int _fg_rgb = 0;
  int _bg_rgb = 0;
};
