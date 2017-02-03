#pragma once

#include <string>
#include <map>
#include <vector>

class Config
{
public:
    Config(const std::string& path, const std::string& name);
    
    std::string get(const std::string& key, const std::string& def) const;
    void set(const std::string& key, const std::string& value, bool bCreateOnly = false);

    bool save();
    std::string name() const;
    std::vector<std::string> keys() const;
private:
    std::string savePath() const;
    std::map<std::string, std::string> _data;
    std::string _path;
    std::string _name;
};
