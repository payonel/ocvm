#pragma once

#include "value.h"

class Config
{
public:
    Config();
    
    Value get(const Value& key) const;
    bool set(const Value& key, const Value& value, bool bCreateOnly = false);

    bool load(const string& path, const string& name);
    bool save();
    string name() const;
    vector<ValuePair> pairs() const;
private:
    string savePath() const;
    Value _data;
    string _path;
    string _name;
};
