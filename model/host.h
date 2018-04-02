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
    std::unique_ptr<Component> create(const string& type) const;
    void close();

    string stackLog() const;
    void stackLog(const string& stack_log);

    string biosPath() const;
    void biosPath(const string& bios_path);

    string fontsPath() const;
    void fontsPath(const string& fonts_path);

    string machinePath() const;
    void machinePath(const string& machine_path);

private:
    string _frameType;
    string _stack_log;
    string _bios_path;
    string _fonts_path;
    string _machine_path;
};
