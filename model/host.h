#pragma once
#include <string>
#include "config.h"

class Component;
class Framer;

class Host
{
public:
    Host(Framer* framer);
    ~Host();

    Framer* getFramer() const;
    Component* create(const string& type);
    void close();

    string stackLog() const { return _stack_log; }
    void stackLog(const string& stack_log) { _stack_log = stack_log; }

private:
    Framer* _framer;
    string _stack_log;
};
