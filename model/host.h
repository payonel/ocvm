#pragma once
#include <string>
#include <memory>
#include <map>

class Component;
class Frame;

class Host
{
public:
    Host(std::string frameType);
    ~Host();

    Frame* createFrame() const;
    std::unique_ptr<Component> create(const std::string& type) const;
    void close();

    std::string stackLog() const;
    void stackLog(const std::string& stack_log);

    std::string biosPath() const;
    void biosPath(const std::string& bios_path);

    std::string fontsPath() const;
    void fontsPath(const std::string& fonts_path);

    std::string machinePath() const;
    void machinePath(const std::string& machine_path);

private:
    std::string _frameType;
    std::string _stack_log;
    std::string _bios_path;
    std::string _fonts_path;
    std::string _machine_path;

    void load_all();
    typedef std::unique_ptr<Component> create_object_t();
    std::map<std::string, create_object_t*> _creators;
};
