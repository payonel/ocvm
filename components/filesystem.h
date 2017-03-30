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
    bool isReadOnly() const;
    bool isTmpfs() const;

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
    int lastModified(lua_State* lua);
    int spaceUsed(lua_State* lua);
    int spaceTotal(lua_State* lua);
    int remove(lua_State* lua);
    int makeDirectory(lua_State* lua);
    int rename(lua_State* lua);
protected:
    bool onInitialize(Value& config) override;
    static string clean(string arg, bool bAbs, bool removeEnd);
    static string relative(const string& requested, const string& full);
    void init(const string& loot);
    fstream* get_handle(lua_State* lua, int* pIndex = nullptr);
private:
    bool _isReadOnly;
    map<int, fstream*> _handles;
    string _src;
    bool _tmpfs;
};
