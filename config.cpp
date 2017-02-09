#include "config.h"
#include "log.h"

#include <lua.hpp>

#include <iostream>
#include <fstream>
#include <sstream>
using std::ifstream;
using std::ofstream;

extern "C"
{
    static int l_cpp_store(lua_State* lua)
    {
        const void* raw = lua_topointer(lua, 1);
        Config* self = const_cast<Config*>(static_cast<const Config*>(raw));
        Value key(lua, 2);
        Value value(lua, 3);
        self->set(key, value);

        lout << self->name() << " config loading [" << key.serialize() << "]: " << value.serialize() << "\n";
        return 0;
    }
}

Config::Config() : _data(Value::table())
{}

bool Config::load(const string& path, const string& name)
{
    // clear out previous values (if any)
    _data = Value::table();
    _path = path;
    _name = name;

    // first check _path, else local for name
    ifstream input;

    lout << _name << ": ";
    input.open(savePath());
    if (input)
    {
        lout << "config loaded from " << savePath() << "\n";
    }
    else
    {
        input.open(name + ".cfg");
        if (input)
            lout << "default config loaded\n";
        else
            lout << "failed to load\n";
    }

    string table;
    char byte;
    while (byte = input.get())
    {
        if (!input)
            break;
        table += byte;
    }
    input.close();

    lout << "config [" << _name << "]: table: " << table;
    lout << endl;

    if (table.empty())
    {
        return true; // ok, just nothing to load
    }

    string loader =
    "for k,v in pairs(" + table + ") do\n"
    "   cpp_store(_this, k, v)\n"
    "end";

    lua_State* lua = luaL_newstate();
    if (luaL_loadstring(lua, loader.c_str()) == 0)
    {
        luaL_openlibs(lua);
        lua_pushcfunction(lua, l_cpp_store);
        lua_setglobal(lua, "cpp_store");
        lua_pushlightuserdata(lua, this);
        lua_setglobal(lua, "_this");
        lua_pcall(lua, 0, LUA_MULTRET, 0);
    }
    lua_close(lua);
    return true;
}

string Config::name() const
{
    return _name;
}

string Config::savePath() const
{
    return _path + "/" + _name + ".cfg";
}

Value Config::get(const Value& key) const
{
    return _data.get(key);
}

bool Config::set(const Value& key, const Value& value, bool bCreateOnly)
{
    if (!bCreateOnly || _data.get(key) == Value::nil)
    {
        _data.set(key, value);
        return true;
    }
    return false;
}

vector<ValuePair> Config::pairs() const
{
    return _data.pairs();
}

bool Config::save()
{
    stringstream ss;
    ss << "{\n";
    for (auto pair : _data.pairs())
    {
        ss << "[";
        ss << pair.first.serialize();
        ss << "] = ";
        ss << pair.second.serialize();
        ss << ",\n";
    }
    ss << "}\n";

    lout << _name << ": config ";
    ofstream output(savePath());
    if (!output)
    {
        lout << "could not save\n";
        return false;
    }
    output << ss.str();
    output.close();
    lout << " saved\n";
    return true;
}
