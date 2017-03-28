#pragma once

#include "io/frame.h"

#include <vector>
#include <map>

using std::map;

struct AnsiFrameState
{
};

class AnsiEscapeTerm : public Framer
{
public:
    AnsiEscapeTerm();
    ~AnsiEscapeTerm();
    bool update() override;
    tuple<int, int> maxResolution() const override;
    void clear() override;
protected:
    bool onOpen() override;
    void onClose() override;
    void onWrite(Frame* pf, int x, int y, const Cell& cell) override;
    bool onAdd(Frame* pf) override;
private:
    map<Frame*, AnsiFrameState> _states;

    int _x = 0;
    int _y = 0;
    int _fg_rgb = 0;
    int _bg_rgb = 0;
};
