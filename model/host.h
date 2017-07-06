#pragma once
#include <string>
#include "config.h"

class Component;
class Frame;

class Host
{
public:
    Host(string frameType);
    ~Host();

    Frame* createFrame() const;
    Component* create(const string& type) const;
    void close();

    string stackLog() const { return _stack_log; }
    void stackLog(const string& stack_log) { _stack_log = stack_log; }

private:
    string _frameType;
    string _stack_log;
};
