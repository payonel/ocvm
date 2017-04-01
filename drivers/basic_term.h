#pragma once

#include "io/frame.h"

class BasicTerm : public Framer
{
public:
    bool update() override;
    tuple<int, int> maxResolution() const override;
protected:
    bool onOpen() override;
    void onClose() override;
    void write(Frame* pf, int x, int y, const Cell& cell) override;
};
