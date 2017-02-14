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
    Filesystem();

    string path() const;
    string src() const;

    ValuePack open(lua_State* lua);
    ValuePack read(lua_State* lua);
    ValuePack close(lua_State* lua);
    ValuePack getLabel(lua_State* lua);
    ValuePack list(lua_State* lua);
    ValuePack isDirectory(lua_State* lua);
    ValuePack exists(lua_State* lua);
    
protected:
    bool onInitialize(Value& config) override;
    static string clean(string arg, bool bAbs, bool removeEnd);
    static string relative(const string& requested, const string& full);
    void init(const string& loot);
private:
    map<int, fstream*> _handles;
    string _src;
};
