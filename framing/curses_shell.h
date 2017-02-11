#pragma once

#include <vector>
#include <map>
#include "frame.h"

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
    bool open() override;
    void close() override;

    Frame* getFrame(int x, int y) const;
protected:
    bool onAdd(Frame* pf);
    void onWrite(Frame* pWhichFrame) override;
    void onResolution(Frame* pWhichFrame) override;
private:
    // termios* _original;
    map<Frame*, FrameState> _states;
};
