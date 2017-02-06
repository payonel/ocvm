#pragma once

#include <vector>
#include <map>
#include "frame.h"

struct _win_st;
typedef _win_st WINDOW;

struct FrameState
{
    WINDOW* window;
};

class Shell : public Framer
{
public:
    Shell();
    ~Shell();
    bool update() override;
    bool open() override;
    void close() override;
    bool add(Frame* pf, size_t index) override;

    Frame* getFrame(int x, int y) const;
protected:
    void onWrite(Frame* pWhichFrame) override;
    void onResolution(Frame* pWhichFrame) override;
private:
    // termios* _original;
    std::map<Frame*, FrameState> _states;
};
