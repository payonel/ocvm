#pragma once

#include "value.h"
#include <string>
#include <map>
#include <vector>

class Config
{
public:
    Config();
    
    Value get(const Value& key) const;
    bool set(const Value& key, const Value& value, bool bCreateOnly = false);

    bool load(const std::string& path, const std::string& name);
    bool save();
    std::string name() const;
    std::vector<ValuePair> pairs() const;
private:
    std::string savePath() const;
    Value _data;
    std::string _path;
    std::string _name;
};
