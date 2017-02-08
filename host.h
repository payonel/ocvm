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
    Component* create(const string& type, const Value& init);
    void close();
private:
    string _env_path;
    Framer* _framer;
};
