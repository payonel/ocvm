#pragma once
#include "component.h"
#include "value.h"

#include <map>
#include <fstream>
using std::map;
using std::fstream;

class Filesystem : public Component
{
public:
    Filesystem(Value& config, Host* host);

    string path() const;
    string src() const;

    ValuePack open(const ValuePack& args);
    ValuePack read(const ValuePack& args);
    ValuePack close(const ValuePack& args);
    ValuePack getLabel(const ValuePack& args);
    ValuePack list(const ValuePack& args);
    ValuePack isDirectory(const ValuePack& args);
    ValuePack exists(const ValuePack& args);
    
protected:
    static string clean(string arg, bool bAbs, bool removeEnd);
    static string relative(const string& requested, const string& full);
    void init(const string& loot);
private:
    map<int, fstream*> _handles;
    string _src;
};
