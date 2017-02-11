#pragma once
#include <string>
#include "config.h"

class Component;
class Framer;

class Host
{
public:
    Host(const string& env_path);
    ~Host();

    string machinePath() const;
    string envPath() const;
    Framer* getFramer() const;
    Component* create(Value& config);
    void close();
    void mkdir(const string& path);
private:
    string _env_path;
    Framer* _framer;
};
