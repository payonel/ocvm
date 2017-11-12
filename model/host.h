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

    string biosPath() const { return _bios_path; }
    void biosPath(const string& bios_path) { _bios_path = bios_path; }

    string fontsPath() const { return _fonts_path; }
    void fontsPath(const string& fonts_path);

    string machinePath() const { return _machine_path; }
    void machinePath(const string& machine_path) { _machine_path = machine_path; }

private:
    string _frameType;
    string _stack_log;
    string _bios_path;
    string _fonts_path;
    string _machine_path;
};
