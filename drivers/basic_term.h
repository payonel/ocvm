#pragma once

#include "io/frame.h"

class BasicTerm : public Framer
{
public:
    bool update() override;
    bool open() override;
    void close() override;
    void onResolution(Frame* pWhichFrame) override;
    tuple<int, int> maxResolution() const override;
protected:
    void onWrite(Frame* pf, int x, int y, const Cell& cell) override;
};
