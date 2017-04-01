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
    void write(Frame* pf, int x, int y, const Cell& cell) override;
protected:
    bool onOpen() override;
    void onClose() override;
    bool onAdd(Frame* pf) override;
    string scrub(const string& value) const;
private:
    map<Frame*, AnsiFrameState> _states;

    int _x = 0;
    int _y = 0;
    int _fg_rgb = 0;
    int _bg_rgb = 0;
};
