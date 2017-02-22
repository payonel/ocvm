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

    int open(lua_State* lua);
    int read(lua_State* lua);
    int write(lua_State* lua);
    int close(lua_State* lua);
    int getLabel(lua_State* lua);
    int list(lua_State* lua);
    int isDirectory(lua_State* lua);
    int exists(lua_State* lua);
    int isReadOnly(lua_State* lua);
    int seek(lua_State* lua);
    int size(lua_State* lua);
    
protected:
    bool onInitialize(Value& config) override;
    static string clean(string arg, bool bAbs, bool removeEnd);
    static string relative(const string& requested, const string& full);
    void init(const string& loot);
    fstream* get_handle(lua_State* lua, int* pIndex = nullptr);
private:
    map<int, fstream*> _handles;
    string _src;
};
