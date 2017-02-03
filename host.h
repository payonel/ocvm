#pragma once
#include <string>

class Component;

class Host
{
public:
    Host(const std::string& env_path);
    std::string machinePath() const;
    std::string envPath() const;
    Component* create(const std::string& type);
    void close();
    ~Host();
private:
    std::string _env_path;
};
