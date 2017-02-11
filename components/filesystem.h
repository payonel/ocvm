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
    Filesystem(const Value& config, Host* host);

    string path() const;

    ValuePack open(const ValuePack& args);
    ValuePack read(const ValuePack& args);
    ValuePack close(const ValuePack& args);
    ValuePack getLabel(const ValuePack& args);
    ValuePack list(const ValuePack& args);
    ValuePack isDirectory(const ValuePack& args);
    
    static string clean(string arg, bool bAbs, bool removeEnd);
protected:
    static string relative(const string& requested, const string& full);
    void init(const string& loot);
private:
    map<int, fstream*> _handles;
};
