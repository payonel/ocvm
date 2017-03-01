#pragma once

#include "frame.h"

#include <vector>
#include <map>
#include "frame.h"

using std::map;

struct AnsiFrameState
{
};

struct termios;

class AnsiEscapeTerm : public Framer
{
public:
    ~AnsiEscapeTerm();
    bool update() override;
    bool open() override;
    void close() override;
    void onResolution(Frame* pWhichFrame) override;
    tuple<int, int> maxResolution() const override;
protected:
    void onWrite(Frame* pf, int x, int y, const Cell& cell) override;
    bool onAdd(Frame* pf) override;
private:
    map<Frame*, AnsiFrameState> _states;
    termios* _original {};
};
