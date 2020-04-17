#pragma once

#include "value.h"

class Logger;

class Config
{
public:
  Config();

  const Value& get(const string& key) const;
  Value& get(const string& key);
  bool set(const string& key, const Value& value, bool bCreateOnly = false);

  bool load(const string& path, const string& name);
  bool save() const;
  string name() const;
  vector<string> keys() const;

private:
  string savePath() const;
  void clear_n(Value& t);
  Value _data;
  string _path;
  string _name;
  string _cache;
};
