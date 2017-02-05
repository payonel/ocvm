#pragma once

#include <vector>
#include <map>
#include "frame.h"

struct termios;

struct FrameState
{
    int left;
    int top;
    int width;
    int height;
    int x;
    int y;
};

class Shell : public Framer
{
public:
    Shell();
    ~Shell();
    bool update() override;
    bool open() override;
    void close() override;
    bool add(Frame* pf) override;

    Frame* getFrame(int x, int y);
protected:
    void onWrite(Frame* pWhichFrame) override;
    void onResolution(Frame* pWhichFrame, int oldw, int oldh) override;
private:
    termios* _original;
    std::map<Frame*, FrameState> _states;
};
