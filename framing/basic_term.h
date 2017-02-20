#pragma once

#include "frame.h"

class BasicTerm : public Framer
{
public:
    bool update() override;
    bool open() override;
    void close() override;
    void onWrite(Frame* pWhichFrame) override;
    void onResolution(Frame* pWhichFrame) override;
    tuple<int, int> maxResolution() const override;
protected:
};
