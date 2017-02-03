#pragma once

class IFrame;

class Shell
{
public:
    Shell();
    ~Shell();
    bool add(IFrame* pframe);
    bool update();
    void close();
};
