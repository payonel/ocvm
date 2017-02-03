#include "config.h"
#include "log.h"

#include <lua.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
using std::string;
using std::ifstream;
using std::ofstream;
using std::stringstream;
using std::vector;

static Config* temp_config = nullptr;

extern "C"
{
    static int l_cpp_store(lua_State* lua)
    {
        if (temp_config)
        {
            const char* ckey = luaL_checkstring(lua, 1);
            const char* cvalue = luaL_checkstring(lua, 2);
            temp_config->set(ckey, cvalue);

            log << temp_config->name() << " config loading [" << ckey << "]: " << cvalue << "\n";
        }

        return 0;
    }
}

Config::Config(const string& path, const string& name) : _path(path), _name(name)
{
    // first check _path, else local for name
    ifstream input;

    log << _name << ": ";
    input.open(savePath());
    if (input)
    {
        log << "config loaded from " << savePath() << "\n";
    }
    else
    {
        input.open(name + ".cfg");
        if (input)
            log << "default config loaded\n";
        else
            log << "failed to load\n";
    }

    string table;
    char byte;
    while (input >> byte)
    {
        table += byte;
    }
    input.close();

    if (table.empty())
    {
        return;
    }

    string loader =
    "for k,v in pairs(" + table + ") do\n"
    "   cpp_store(k, v)\n"
    "end";

    lua_State* lua = luaL_newstate();
    if (luaL_loadstring(lua, loader.c_str()) == 0)
    {
        luaL_openlibs(lua);
        lua_pushcfunction(lua, l_cpp_store);
        lua_setglobal(lua, "cpp_store");
        temp_config = this;
        lua_pcall(lua, 0, LUA_MULTRET, 0);
        temp_config = nullptr;
    }
    lua_close(lua);
}

string Config::name() const
{
    return _name;
}

string Config::savePath() const
{
    return _path + "/" + _name + ".cfg";
}

string Config::get(const string& key, const string& def) const
{
    if (_data.find(key) != _data.end())
        return _data.at(key);
    else
        return def;
}

void Config::set(const string& key, const string& value, bool bCreateOnly)
{
    if (!bCreateOnly || _data.find(key) == _data.end())
    {
        _data[key] = value;
    }
}

vector<string> Config::keys() const
{
    vector<string> k;
    for (auto pair : _data)
        k.push_back(pair.first);

    return k;
}

bool Config::save()
{
    stringstream ss;
    ss << "{\n";
    for (auto pair : _data)
    {
        ss << "[\"";
        ss << pair.first;
        ss << "\"] = \"";
        ss << pair.second;
        ss << "\",\n";
    }
    ss << "}\n";

    log << _name << ": config ";
    ofstream output(savePath());
    if (!output)
    {
        log << "could not save\n";
        return false;
    }
    output << ss.str();
    output.close();
    log << " saved\n";
    return true;
}

