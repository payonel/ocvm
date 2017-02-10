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
    Filesystem(const string& type, const Value& init, Host* host);

    string path() const;

    ValuePack open(const ValuePack& args);
    ValuePack read(const ValuePack& args);
    ValuePack close(const ValuePack& args);
    ValuePack getLabel(const ValuePack& args);
protected:
    void init(const string& loot);
private:
    map<int, fstream*> _handles;
};
