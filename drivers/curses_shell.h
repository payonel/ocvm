#pragma once

#include <vector>
#include <map>
#include "io/frame.h"

using std::map;

struct _win_st;
typedef _win_st WINDOW;

struct FrameState
{
    WINDOW* window;
};

class CursesShell : public Framer
{
public:
    CursesShell();
    ~CursesShell();
    bool update() override;

    Frame* getFrame(int x, int y) const;
    tuple<int, int> maxResolution() const override;
protected:
    bool onOpen() override;
    void onClose() override;
    bool onAdd(Frame* pf);
    void onWrite(Frame* pf, int x, int y, const Cell& cell) override;
    void onResolution(Frame* pWhichFrame) override;
private:
    // termios* _original;
    map<Frame*, FrameState> _states;
};
